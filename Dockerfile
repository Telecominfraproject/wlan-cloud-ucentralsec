FROM alpine AS builder

RUN apk add --update --no-cache \
    openssl openssh \
    ncurses-libs \
    bash util-linux coreutils curl \
    make cmake gcc g++ libstdc++ libgcc git zlib-dev \
    openssl-dev boost-dev unixodbc-dev postgresql-dev mariadb-dev \
    apache2-utils yaml-dev apr-util-dev \
    librdkafka-dev

RUN git clone https://github.com/stephb9959/poco /poco
RUN git clone https://github.com/stephb9959/cppkafka /cppkafka

WORKDIR /cppkafka
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8
RUN cmake --build . --target install

WORKDIR /poco
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8
RUN cmake --build . --target install

ADD CMakeLists.txt build /owsec/
ADD cmake /owsec/cmake
ADD src /owsec/src

WORKDIR /owsec
RUN mkdir cmake-build
WORKDIR /owsec/cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8

FROM alpine

ENV OWSEC_USER=owsec \
    OWSEC_ROOT=/owsec-data \
    OWSEC_CONFIG=/owsec-data

RUN addgroup -S "$OWSEC_USER" && \
    adduser -S -G "$OWSEC_USER" "$OWSEC_USER"

RUN mkdir /openwifi
RUN mkdir -p "$OWSEC_ROOT" "$OWSEC_CONFIG" && \
    chown "$OWSEC_USER": "$OWSEC_ROOT" "$OWSEC_CONFIG"
RUN apk add --update --no-cache librdkafka mariadb-connector-c libpq unixodbc su-exec gettext ca-certificates
COPY --from=builder /ucentralsec/cmake-build/ucentralsec /ucentral/ucentralsec
COPY --from=builder /cppkafka/cmake-build/src/lib/* /lib/
COPY --from=builder /poco/cmake-build/lib/* /lib/

COPY owsec.properties.tmpl ${OWSEC_CONFIG}/
COPY docker-entrypoint.sh /
RUN wget https://raw.githubusercontent.com/Telecominfraproject/wlan-cloud-ucentral-deploy/main/docker-compose/certs/restapi-ca.pem \
    -O /usr/local/share/ca-certificates/restapi-ca-selfsigned.pem

EXPOSE 16001 17001 16101

ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["/openwifi/owsec"]
