#include "Framework.h"

enum ErrorCode {
    EC_SUCCCES = 0,
    EC_FAILURE = 1,
};

TEST(Foo) {
    return true;
}

int main() {
    if (!aby::TestFramework::get().run()) {
        return EC_FAILURE;
    }
    return EC_SUCCCES;
}