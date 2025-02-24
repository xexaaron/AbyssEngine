#include "Framework.h"

enum ErrorCode {
    EC_SUCCCES = 0,
    EC_FAILURE = 1,
};

int main() {
    if (!aby::TestFramework::get().run()) {
        return EC_FAILURE;
    }
    return EC_SUCCCES;
}