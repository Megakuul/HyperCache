FROM archlinux:latest as builder

RUN pacman -Sy --noconfirm
RUN pacman -S --noconfirm base-devel clang cmake python-pip
RUN pip install conan

COPY . /usr/src/hc_prox
WORKDIR /usr/src/hc_prox

RUN conan install . --build=missing
RUN mkdir build && cd build && cmake -D STATIC=true ..
RUN cmake --build .

FROM alpine:latest
COPY --from=builder /usr/src/hc_prox/build/hc_prox /usr/bin/hc_prox
ENTRYPOINT ["/usr/bin/hc_prox"]