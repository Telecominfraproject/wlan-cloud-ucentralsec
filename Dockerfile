FROM alpine AS builder

RUN apk update && \
    apk add --no-cache openssl openssh && \
    apk add --no-cache ncurses-libs && \
    apk add --no-cache bash util-linux coreutils curl && \
    apk add --no-cache make cmake gcc g++ libstdc++ libgcc git zlib-dev && \
    apk add --no-cache openssl-dev boost-dev unixodbc-dev postgresql-dev mariadb-dev && \
    apk add --no-cache librdkafka-dev

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

ADD CMakeLists.txt /ucentralsec/
ADD cmake /ucentralsec/cmake
ADD src /ucentralsec/src

WORKDIR /ucentralsec
RUN mkdir cmake-build
WORKDIR /ucentralsec/cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8

FROM alpine

RUN mkdir /ucentral
RUN mkdir /ucentralsec-data
RUN apk add --update --no-cache librdkafka mariadb-connector-c libpq unixodbc

COPY --from=builder /ucentralsec/cmake-build/ucentralsec /ucentral/ucentralsec
COPY --from=builder /cppkafka/cmake-build/src/lib/* /lib/
COPY --from=builder /poco/cmake-build/lib/* /lib/

EXPOSE 16001
EXPOSE 17001
EXPOSE 16101

ENTRYPOINT /ucentral/ucentralsec
