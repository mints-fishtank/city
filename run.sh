#!/bin/bash

# City - Launch Script
# Usage: ./run.sh [command] [options]

BUILD_DIR="build/debug"
SERVER_BIN="$BUILD_DIR/src/server/city_server"
CLIENT_BIN="$BUILD_DIR/src/client/city_client"
DEFAULT_PORT=9999

show_help() {
    echo "City Launch Script"
    echo ""
    echo "Usage: ./run.sh <command> [options]"
    echo ""
    echo "Commands:"
    echo "  server [port]           Start the server (default port: $DEFAULT_PORT)"
    echo "  client [name]           Start client in standalone mode"
    echo "  connect [name] [host]   Connect client to server"
    echo "  both [name]             Start server + connected client"
    echo "  build                   Build both server and client"
    echo "  help                    Show this help"
    echo ""
    echo "Examples:"
    echo "  ./run.sh server              # Start server on port $DEFAULT_PORT"
    echo "  ./run.sh server 7777         # Start server on port 7777"
    echo "  ./run.sh client              # Standalone client"
    echo "  ./run.sh client Alice        # Standalone with name"
    echo "  ./run.sh connect Bob         # Connect to localhost:$DEFAULT_PORT"
    echo "  ./run.sh connect Bob 192.168.1.5:7777"
    echo "  ./run.sh both Charlie        # Server + client together"
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
    echo "Starting server on port $port..."
    exec "$SERVER_BIN" "$port"
}

cmd_client() {
    local name="${1:-Player}"
    check_build
    echo "Starting client in standalone mode..."
    exec "$CLIENT_BIN" --name "$name"
}

cmd_connect() {
    local name="${1:-Player}"
    local host="${2:-localhost:$DEFAULT_PORT}"
    check_build
    echo "Connecting to $host as $name..."
    exec "$CLIENT_BIN" --connect "$host" --name "$name"
}

cmd_both() {
    local name="${1:-Player}"
    check_build

    echo "Starting server on port $DEFAULT_PORT..."
    "$SERVER_BIN" "$DEFAULT_PORT" &
    SERVER_PID=$!

    # Give server time to start
    sleep 1

    echo "Starting client as $name..."
    "$CLIENT_BIN" --connect "localhost:$DEFAULT_PORT" --name "$name"

    # Clean up server when client exits
    kill $SERVER_PID 2>/dev/null
    wait $SERVER_PID 2>/dev/null
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
    both)    cmd_both "$2" ;;
    build)   cmd_build ;;
    help|--help|-h) show_help ;;
    *)
        echo "Unknown command: $1"
        echo "Run './run.sh help' for usage"
        exit 1
        ;;
esac
