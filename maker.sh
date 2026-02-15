#!/bin/sh

debug() {
    set -o xtrace
    cmake -B Build -D CMAKE_BUILD_TYPE=Debug &&
        cmake --build ./Build --parallel
}

release() {
    set -o xtrace
    cmake -B Build -D CMAKE_BUILD_TYPE=Release &&
        cmake --build ./Build --parallel
}

clean() {
    set -o xtrace
    rm -rf ./Build
}

zip() {
    git archive --format zip -o Leontiev-graphics.zip HEAD
}

help() {
    echo "Использование:"
    echo "./maker.sh release -> Собрать проект с помощью CMake и make -C ./Build/"
    echo "./maker.sh debug -> Собрать проект с сохранением информации для отладки"
    echo "./maker.sh clean -> Удалить содержимое директории ./Build/"
}

case "$1" in
debug) debug ;;
release) release ;;
clean) clean ;;
zip) zip ;;
*) help ;;
esac
