ARG DEBIAN_VERSION=11.5-slim
ARG POCO_VERSION=poco-tip-v1
ARG CPPKAFKA_VERSION=tip-v1

FROM debian:$DEBIAN_VERSION AS build-base

RUN apt-get update && apt-get install --no-install-recommends -y \
    make cmake g++ git curl zip unzip pkg-config \
    libpq-dev libmariadb-dev libmariadbclient-dev-compat \
    librdkafka-dev libboost-all-dev libssl-dev \
    zlib1g-dev ca-certificates libcurl4-openssl-dev libfmt-dev

FROM build-base AS poco-build

ARG POCO_VERSION

ADD https://api.github.com/repos/AriliaWireless/poco/git/refs/tags/${POCO_VERSION} version.json
RUN git clone https://github.com/AriliaWireless/poco --branch ${POCO_VERSION} /poco

WORKDIR /poco
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8
RUN cmake --build . --target install

FROM build-base AS cppkafka-build

ARG CPPKAFKA_VERSION

ADD https://api.github.com/repos/AriliaWireless/cppkafka/git/refs/tags/${CPPKAFKA_VERSION} version.json
RUN git clone https://github.com/AriliaWireless/cppkafka --branch ${CPPKAFKA_VERSION} /cppkafka

WORKDIR /cppkafka
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8
RUN cmake --build . --target install

FROM build-base AS owsec-build

ADD CMakeLists.txt build /owsec/
ADD overlays /owsec/overlays
ADD cmake /owsec/cmake
ADD src /owsec/src
ADD .git /owsec/.git
RUN git clone https://github.com/microsoft/vcpkg && \
    ./vcpkg/bootstrap-vcpkg.sh && \
    mkdir /vcpkg/custom-triplets && \
    cp /vcpkg/triplets/x64-linux.cmake /vcpkg/custom-triplets/x64-linux.cmake && \
    sed -i 's/set(VCPKG_LIBRARY.*/set(VCPKG_LIBRARY_LINKAGE dynamic)/g' /vcpkg/custom-triplets/x64-linux.cmake && \
    ./vcpkg/vcpkg install aws-sdk-cpp[sns]:x64-linux json-schema-validator:x64-linux --overlay-triplets=/vcpkg/custom-triplets --overlay-ports=/owsec/overlays

COPY --from=poco-build /usr/local/include /usr/local/include
COPY --from=poco-build /usr/local/lib /usr/local/lib
COPY --from=cppkafka-build /usr/local/include /usr/local/include
COPY --from=cppkafka-build /usr/local/lib /usr/local/lib

WORKDIR /owsec
RUN mkdir cmake-build
WORKDIR /owsec/cmake-build
RUN cmake -DCMAKE_TOOLCHAIN_FILE=/vcpkg/scripts/buildsystems/vcpkg.cmake ..
RUN cmake --build . --config Release -j8

FROM debian:$DEBIAN_VERSION

ENV OWSEC_USER=owsec \
    OWSEC_ROOT=/owsec-data \
    OWSEC_CONFIG=/owsec-data

RUN useradd "$OWSEC_USER"

RUN mkdir /openwifi
RUN mkdir -p "$OWSEC_ROOT" "$OWSEC_CONFIG" && \
    chown "$OWSEC_USER": "$OWSEC_ROOT" "$OWSEC_CONFIG"

RUN apt-get update && apt-get install --no-install-recommends -y \
    librdkafka++1 gosu gettext ca-certificates bash jq curl wget \
    libmariadb-dev-compat libpq5 unixodbc postgresql-client libfmt7

COPY readiness_check /readiness_check
COPY test_scripts/curl/cli /cli

COPY owsec.properties.tmpl /
COPY wwwassets /dist/wwwassets
COPY templates /dist/templates
COPY docker-entrypoint.sh /
COPY wait-for-postgres.sh /
RUN wget https://raw.githubusercontent.com/Telecominfraproject/wlan-cloud-ucentral-deploy/main/docker-compose/certs/restapi-ca.pem \
    -O /usr/local/share/ca-certificates/restapi-ca-selfsigned.crt

COPY --from=owsec-build /owsec/cmake-build/owsec /openwifi/owsec
COPY --from=owsec-build /vcpkg/installed/x64-linux/lib/ /usr/local/lib/
COPY --from=cppkafka-build /cppkafka/cmake-build/src/lib/ /usr/local/lib/
COPY --from=poco-build /poco/cmake-build/lib/ /usr/local/lib/

RUN ldconfig

EXPOSE 16001 17001 16101

ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["/openwifi/owsec"]
