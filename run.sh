#!/bin/bash

# City - Launch Script
# Usage: ./run.sh [command] [options]

BUILD_DIR="build/debug"
SERVER_BIN="$BUILD_DIR/src/server/city_server"
CLIENT_BIN="$BUILD_DIR/src/client/city_client"
DEFAULT_PORT=7777

show_help() {
    echo "City Launch Script"
    echo ""
    echo "Usage: ./run.sh <command> [options]"
    echo ""
    echo "Commands:"
    echo "  server [port]           Start dedicated server (default port: $DEFAULT_PORT)"
    echo "  client [name]           Start client (creates local server automatically)"
    echo "  connect [name] [host]   Connect to external server"
    echo "  build                   Build both server and client"
    echo "  help                    Show this help"
    echo ""
    echo "Examples:"
    echo "  ./run.sh client              # Single player (local server)"
    echo "  ./run.sh client Alice        # Single player with name"
    echo "  ./run.sh server              # Start dedicated server for others to join"
    echo "  ./run.sh connect Bob         # Join server at localhost:$DEFAULT_PORT"
    echo "  ./run.sh connect Bob 192.168.1.5:7777  # Join remote server"
}

check_build() {
    if [ ! -f "$SERVER_BIN" ] || [ ! -f "$CLIENT_BIN" ]; then
        echo "Binaries not found. Building..."
        cmake --build "$BUILD_DIR" --target city_server city_client
        if [ $? -ne 0 ]; then
            echo "Build failed!"
            exit 1
        fi
    fi
}

cmd_server() {
    local port="${1:-$DEFAULT_PORT}"
    check_build
    echo "Starting dedicated server on port $port..."
    exec "$SERVER_BIN" "$port"
}

cmd_client() {
    local name="${1:-Player}"
    check_build
    echo "Starting client as $name (local server)..."
    exec "$CLIENT_BIN" --name "$name"
}

cmd_connect() {
    local name="${1:-Player}"
    local host="${2:-localhost:$DEFAULT_PORT}"
    check_build
    echo "Connecting to $host as $name..."
    exec "$CLIENT_BIN" --connect "$host" --name "$name"
}

cmd_build() {
    echo "Building server and client..."
    cmake --build "$BUILD_DIR" --target city_server city_client
}

# Main
case "${1:-help}" in
    server)  cmd_server "$2" ;;
    client)  cmd_client "$2" ;;
    connect) cmd_connect "$2" "$3" ;;
    build)   cmd_build ;;
    help|--help|-h) show_help ;;
    *)
        echo "Unknown command: $1"
        echo "Run './run.sh help' for usage"
        exit 1
        ;;
esac
