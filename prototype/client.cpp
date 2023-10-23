#include <iostream>
#include <winsock2.h>
#include "Lat.cpp" // You should have your LDX class definition in a header file for reuse



int main() {
    // Example usage (ensure you have the necessary data to feed into the functions):
    unordered_map<string, vector<string>> MM = {
        {"label1", {"value1", "value2"}},
        {"label2", {"value3", "value4", "value5"}}
    };

    LAT lat(10, MM);
    vector<string> result = lat.GetC_P("label2");
   
    return 0;
}
