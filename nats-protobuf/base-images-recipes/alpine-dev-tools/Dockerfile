FROM cmuramoto/alpine-base:1.0

#default tar does not support -I to unpack with zstd
RUN apk add --no-cache alpine-sdk alsa-lib alsa-lib-dev autoconf \
    bash cups-dev cups-libs fontconfig fontconfig-dev freetype \
    freetype-dev grep libx11 libx11-dev libxext libxext-dev \
    libxrandr libxrandr-dev libxrender libxrender-dev libxt \
    libxt-dev libxtst libxtst-dev linux-headers zip
