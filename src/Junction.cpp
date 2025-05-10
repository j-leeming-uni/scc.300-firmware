#include "Junction.h"

#include <Arduino.h>
#include <Ethernet.h>

#include "config.h"

Neighbour::T Neighbour::next(const T neighbour) {
    switch (neighbour) {
        case NORTH:
            return EAST;
        case SOUTH:
            return WEST;
        case EAST:
            return SOUTH;
        case WEST:
            return NORTH;
        default:;
    }
    return NONE;
}

TrafficFlow::T Neighbour::get_flow_for(const T neighbour) {
    switch (neighbour) {
        case NORTH:
            return TrafficFlow::NORTH;
        case SOUTH:
            return TrafficFlow::SOUTH;
        case EAST:
            return TrafficFlow::EAST;
        case WEST:
            return TrafficFlow::WEST;
        default:;
    }
    return TrafficFlow::NONE;
}

TrafficFlow::T TrafficFlow::next(T flow) {
#ifdef FLIPPED
    switch (flow) {
        case NORTH_SOUTH:
            return EAST_WEST;
        case EAST_WEST:
            return NORTH_SOUTH;
        case NORTH:
            return WEST;
        case SOUTH:
            return EAST;
        case EAST:
            return NORTH;
        case WEST:
            return SOUTH;
        default:
            return NONE;
    }
#else
    switch (flow) {
        case TrafficFlow::NORTH_SOUTH:
            return TrafficFlow::EAST_WEST;
        case TrafficFlow::EAST_WEST:
            return TrafficFlow::NORTH_SOUTH;
        case TrafficFlow::NORTH:
            return TrafficFlow::EAST;
        case TrafficFlow::SOUTH:
            return TrafficFlow::WEST;
        case TrafficFlow::EAST:
            return TrafficFlow::SOUTH;
        case TrafficFlow::WEST:
            return TrafficFlow::NORTH;
        default:
            return TrafficFlow::NONE;
    }
#endif
}

TrafficFlow::T TrafficFlow::get_opposite(const T flow) {
    switch (flow) {
        case NORTH_SOUTH:
            return NORTH_SOUTH;
        case EAST_WEST:
            return EAST_WEST;
        case NORTH:
            return SOUTH;
        case SOUTH:
            return NORTH;
        case EAST:
            return WEST;
        case WEST:
            return EAST;
        default:
            return NONE;
    }
}


TrafficFlow::T TrafficFlow::with_opposite(const T flow) {
    switch (flow) {
        case NORTH_SOUTH:
            return NORTH_SOUTH;
        case EAST_WEST:
            return EAST_WEST;
        case NORTH:
        case SOUTH:
            return NORTH_SOUTH;
        case EAST:
        case WEST:
            return EAST_WEST;
        default:
            return NONE;
    }
}

void TrafficFlow::to_array(const T flow, bool arr[4]) {
    switch (flow) {
        case NORTH_SOUTH:
            arr[0] = true;
            arr[1] = false;
            arr[2] = true;
            arr[3] = false;
            break;
        case EAST_WEST:
            arr[0] = false;
            arr[1] = true;
            arr[2] = false;
            arr[3] = true;
            break;
        case NORTH:
            arr[0] = true;
            arr[1] = false;
            arr[2] = false;
            arr[3] = false;
            break;
        case SOUTH:
            arr[0] = false;
            arr[1] = false;
            arr[2] = true;
            arr[3] = false;
            break;
        case EAST:
            arr[0] = false;
            arr[1] = true;
            arr[2] = false;
            arr[3] = false;
            break;
        case WEST:
            arr[0] = false;
            arr[1] = false;
            arr[2] = false;
            arr[3] = true;
            break;
        default:
            break;
    }
}


JunctionState JunctionState::from(const uint8_t x) {
    return JunctionState{.id = static_cast<unsigned char>((x & 0xf0) >> 4),
                         .north = static_cast<unsigned char>((x & 0x08) >> 3),
                         .south = static_cast<unsigned char>((x & 0x04) >> 2),
                         .east = static_cast<unsigned char>((x & 0x02) >> 1),
                         .west = static_cast<unsigned char>((x & 0x01) >> 0)};
}

template<>
char JunctionState::to() {
    return id << 4 | north << 3 | south << 2 | east << 1 | west << 0;
}

IPAddress JunctionState::getIP() const { return {192, 168, 2, id}; }

Junction::Junction(const uint8_t id, const uint8_t north, const uint8_t south, const uint8_t east, const uint8_t west) {
#if NON_REPUDIATION == 1
    publicKey = id;
    privateKey = 8 - id;
#endif
    state.id = id;
    neighbours[Neighbour::NORTH].id = north;
    neighbours[Neighbour::SOUTH].id = south;
    neighbours[Neighbour::EAST].id = east;
    neighbours[Neighbour::WEST].id = west;
}

IPAddress Junction::getIP() const { return state.getIP(); }

bool Junction::incoming(const Neighbour::T neighbour) const {
    switch (neighbour) {
        case Neighbour::NORTH:
            return neighbours[Neighbour::NORTH].south;
        case Neighbour::SOUTH:
            return neighbours[Neighbour::SOUTH].north;
        case Neighbour::EAST:
            return neighbours[Neighbour::EAST].west;
        case Neighbour::WEST:
            return neighbours[Neighbour::WEST].east;
        default:
            return false;
    }
}

void Junction::setFlow(const TrafficFlow::T flow) {
    switch (flow) {
        case TrafficFlow::NORTH_SOUTH:
            state.north = 1;
            state.south = 1;
            state.east = 0;
            state.west = 0;
            break;
        case TrafficFlow::EAST_WEST:
            state.north = 0;
            state.south = 0;
            state.east = 1;
            state.west = 1;
            break;
        case TrafficFlow::NORTH:
            state.north = 1;
            state.south = 0;
            state.east = 0;
            state.west = 0;
            break;
        case TrafficFlow::SOUTH:
            state.north = 0;
            state.south = 1;
            state.east = 0;
            state.west = 0;
            break;
        case TrafficFlow::EAST:
            state.north = 0;
            state.south = 0;
            state.east = 1;
            state.west = 0;
            break;
        case TrafficFlow::WEST:
            state.north = 0;
            state.south = 0;
            state.east = 0;
            state.west = 1;
            break;
        default:
            state.north = 0;
            state.south = 0;
            state.east = 0;
            state.west = 0;
            break;
    }
}

void Junction::update_neighbour(const JunctionState state) {
    for (auto &neighbour: neighbours) {
        if (neighbour.id == state.id) {
            neighbour = state;
            break;
        }
    }
}

void Junction::notify_neighbour(const Neighbour::T neighbour) {
    const JunctionState neighbourState = neighbours[neighbour];
    if (neighbourState.id == 0) {
        return;
    }
    const IPAddress ip = neighbourState.getIP();
    EthernetClient client;
    if (!client.connect(ip, SMART_CONNECTION_PORT)) {
        digitalWrite(LEDR, HIGH);
        delay(250);
        digitalWrite(LEDR, LOW);
        return;
    }
    const char message = state.to<char>();
    client.write(message);

#if NON_REPUDIATION == 1
    const char signature = sign(message);
    client.write(publicKey);
    client.write(signature);
#endif
}

void Junction::notify_neighbours() {
    for (int i = 0; i < 4; i++) {
        notify_neighbour(static_cast<Neighbour::T>(i));
    }
}


#if NON_REPUDIATION == 1
constexpr uint8_t rotl(const uint8_t value, const uint8_t count) {
    uint32_t buf = value;
    buf <<= count;
    const uint16_t overflow = buf & 0xff00;
    buf &= 0x00ff;
    buf |= overflow >> 8;
    return buf;
}

char Junction::sign(const char message) const { return rotl(message, privateKey); }

bool Junction::verify(const char message, const char signature, const uint8_t publicKey) {
    return rotl(signature, publicKey) == message;
}
#endif
