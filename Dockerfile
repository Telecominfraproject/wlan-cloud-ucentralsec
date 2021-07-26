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

ADD CMakeLists.txt build /ucentralsec/
ADD cmake /ucentralsec/cmake
ADD src /ucentralsec/src

WORKDIR /ucentralsec
RUN mkdir cmake-build
WORKDIR /ucentralsec/cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8

FROM alpine

ENV UCENTRALSEC_USER=ucentralsec \
    UCENTRALSEC_ROOT=/ucentralsec-data \
    UCENTRALSEC_CONFIG=/ucentralsec-data

RUN addgroup -S "$UCENTRALSEC_USER" && \
    adduser -S -G "$UCENTRALSEC_USER" "$UCENTRALSEC_USER"

RUN mkdir /ucentral
RUN mkdir -p "$UCENTRALSEC_ROOT" "$UCENTRALSEC_CONFIG"
RUN apk add --update --no-cache librdkafka mariadb-connector-c libpq unixodbc su-exec

COPY --from=builder /ucentralsec/cmake-build/ucentralsec /ucentral/ucentralsec
COPY --from=builder /cppkafka/cmake-build/src/lib/* /lib/
COPY --from=builder /poco/cmake-build/lib/* /lib/

EXPOSE 16001 17001 16101

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["/ucentral/ucentralsec"]
