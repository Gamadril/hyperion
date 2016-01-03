#include "Poco/Timer.h"

class MyDataType {
public:
    template<typename T>
    MyDataType &operator[](const T &n) {
        return getAt(n);
    }

private:
    std::vector<MyDataType> _vector;

    MyDataType& getAt(const std::string& n) {
        return _vector[0];
    }
};

int main() {
    MyDataType data;
    MyDataType test = data[0]["test2"];
    return 0;
}