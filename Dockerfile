ARG DEBIAN_VERSION=11.4-slim
ARG POCO_VERSION=poco-tip-v1
ARG FMTLIB_VERSION=9.0.0
ARG CPPKAFKA_VERSION=tip-v1
ARG JSON_VALIDATOR_VERSION=2.1.0
ARG AWS_SDK_VERSION=1.9.315

FROM debian:$DEBIAN_VERSION AS build-base

RUN apt-get update && apt-get install --no-install-recommends -y \
    make cmake g++ git \
    unixodbc-dev libpq-dev libmariadb-dev libmariadbclient-dev-compat \
    librdkafka-dev libboost-all-dev libssl-dev \
    zlib1g-dev nlohmann-json3-dev ca-certificates libcurl4-openssl-dev

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

FROM build-base AS fmtlib-build

ARG FMTLIB_VERSION

ADD https://api.github.com/repos/fmtlib/fmt/git/refs/tags/${FMTLIB_VERSION} version.json
RUN git clone https://github.com/fmtlib/fmt --branch ${FMTLIB_VERSION} /fmtlib

WORKDIR /fmtlib
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN make
RUN make install

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

FROM build-base AS json-schema-validator-build

ARG JSON_VALIDATOR_VERSION

ADD https://api.github.com/repos/pboettch/json-schema-validator/git/refs/tags/${JSON_VALIDATOR_VERSION} version.json
RUN git clone https://github.com/pboettch/json-schema-validator --branch ${JSON_VALIDATOR_VERSION} /json-schema-validator

WORKDIR /json-schema-validator
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN make
RUN make install

FROM build-base AS aws-sdk-cpp-build

ARG AWS_SDK_VERSION

ADD https://api.github.com/repos/aws/aws-sdk-cpp/git/refs/tags/${AWS_SDK_VERSION} version.json
RUN git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp --branch ${AWS_SDK_VERSION} /aws-sdk-cpp

WORKDIR /aws-sdk-cpp
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake .. -DBUILD_ONLY="sns;s3" \
             -DCMAKE_BUILD_TYPE=Release \
             -DCMAKE_CXX_FLAGS="-Wno-error=stringop-overflow -Wno-error=uninitialized" \
             -DAUTORUN_UNIT_TESTS=OFF
RUN cmake --build . --config Release -j8
RUN cmake --build . --target install

FROM build-base AS owsec-build

ADD CMakeLists.txt build /owsec/
ADD cmake /owsec/cmake
ADD src /owsec/src
ADD .git /owsec/.git

COPY --from=poco-build /usr/local/include /usr/local/include
COPY --from=poco-build /usr/local/lib /usr/local/lib
COPY --from=cppkafka-build /usr/local/include /usr/local/include
COPY --from=cppkafka-build /usr/local/lib /usr/local/lib
COPY --from=json-schema-validator-build /usr/local/include /usr/local/include
COPY --from=json-schema-validator-build /usr/local/lib /usr/local/lib
COPY --from=aws-sdk-cpp-build /usr/local/include /usr/local/include
COPY --from=aws-sdk-cpp-build /usr/local/lib /usr/local/lib

COPY --from=fmtlib-build /usr/local/include /usr/local/include
COPY --from=fmtlib-build /usr/local/lib /usr/local/lib

WORKDIR /owsec
RUN mkdir cmake-build
WORKDIR /owsec/cmake-build
RUN cmake .. \
          -Dcrypto_LIBRARY=/usr/lib/libcrypto.so \
          -DBUILD_SHARED_LIBS=ON
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
    libmariadb-dev-compat libpq5 unixodbc

COPY readiness_check /readiness_check
COPY test_scripts/curl/cli /cli

COPY owsec.properties.tmpl /
COPY wwwassets /dist/wwwassets
COPY templates /dist/templates
COPY docker-entrypoint.sh /
COPY wait-for-postgres.sh /
RUN wget https://raw.githubusercontent.com/Telecominfraproject/wlan-cloud-ucentral-deploy/main/docker-compose/certs/restapi-ca.pem \
    -O /usr/local/share/ca-certificates/restapi-ca-selfsigned.pem

COPY --from=owsec-build /owsec/cmake-build/owsec /openwifi/owsec
COPY --from=cppkafka-build /cppkafka/cmake-build/src/lib/* /usr/local/lib
COPY --from=poco-build /poco/cmake-build/lib/* /usr/local/lib
COPY --from=aws-sdk-cpp-build /aws-sdk-cpp/cmake-build/aws-cpp-sdk-core/libaws-cpp-sdk-core.so /usr/local/lib
COPY --from=aws-sdk-cpp-build /aws-sdk-cpp/cmake-build/aws-cpp-sdk-s3/libaws-cpp-sdk-s3.so /usr/local/lib
COPY --from=aws-sdk-cpp-build /aws-sdk-cpp/cmake-build/aws-cpp-sdk-sns/libaws-cpp-sdk-sns.so /usr/local/lib

EXPOSE 16001 17001 16101

ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["/openwifi/owsec"]
