FROM cpp-build-base:0.0.1 AS build

WORKDIR /

RUN echo "HERE IS $PWD"

RUN mkdir vendor && mkdir src && cd vendor && mkdir rapidjson && cd ..

COPY CMakeLists.txt ./
COPY ./src/ ./src/

COPY ./vendor/rapidjson.cmake ./vendor/

# RUN mkdir output && cd output && cmake .. && cmake --build . && cd ..

# FROM ubuntu:bionic

# WORKDIR /opt/zerok

# COPY --from=build /src/zerok ./

# CMD ["./zerok"]