#!/bin/bash

docker build --no-cache -f docker/Dockerfile.alpine.static . --tag=cmuramoto/platao-c-server:1.0-static-alpine
