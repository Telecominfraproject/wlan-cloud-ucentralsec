FROM alpine:3.15 AS build-base

RUN apk add --update --no-cache \
    make cmake g++ git \
    unixodbc-dev postgresql-dev mariadb-dev \
    librdkafka-dev boost-dev openssl-dev \
    zlib-dev nlohmann-json \
    curl-dev

FROM build-base AS poco-build

ADD https://api.github.com/repos/AriliaWireless/poco/git/refs/tags/poco-tip-v1 version.json
RUN git clone https://github.com/AriliaWireless/poco --branch poco-tip-v1 /poco

WORKDIR /poco
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8
RUN cmake --build . --target install

FROM build-base AS fmtlib-build

ADD https://api.github.com/repos/fmtlib/fmt/git/refs/heads/master version.json
RUN git clone https://github.com/fmtlib/fmt /fmtlib

WORKDIR /fmtlib
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN make
RUN make install

FROM build-base AS cppkafka-build

ADD https://api.github.com/repos/stephb9959/cppkafka/git/refs/heads/master version.json
RUN git clone https://github.com/stephb9959/cppkafka /cppkafka

WORKDIR /cppkafka
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8
RUN cmake --build . --target install

FROM build-base AS json-schema-validator-build

ADD https://api.github.com/repos/pboettch/json-schema-validator/git/refs/heads/master version.json
RUN git clone https://github.com/pboettch/json-schema-validator /json-schema-validator

WORKDIR /json-schema-validator
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN make
RUN make install

FROM build-base AS aws-sdk-cpp-build

ADD https://api.github.com/repos/aws/aws-sdk-cpp/git/refs/heads/main version.json
RUN git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp /aws-sdk-cpp

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

FROM alpine:3.15

ENV OWSEC_USER=owsec \
    OWSEC_ROOT=/owsec-data \
    OWSEC_CONFIG=/owsec-data

RUN addgroup -S "$OWSEC_USER" && \
    adduser -S -G "$OWSEC_USER" "$OWSEC_USER"

RUN mkdir /openwifi
RUN mkdir -p "$OWSEC_ROOT" "$OWSEC_CONFIG" && \
    chown "$OWSEC_USER": "$OWSEC_ROOT" "$OWSEC_CONFIG"

RUN apk add --update --no-cache librdkafka su-exec gettext ca-certificates bash jq curl \
    mariadb-connector-c libpq unixodbc postgresql-client

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
