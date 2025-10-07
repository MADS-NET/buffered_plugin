#include "acquisitor.hpp"
#include <iostream>
#include <chrono>
#include <date/date.h>
#include <date/tz.h>

using namespace std;
using namespace std::chrono;
using namespace date;
using json = nlohmann::json;

int main() {
  json j;
  j["capacity"] = 10;
  j["mean"] = 10;
  j["sd"] = 2;

  Acquisitor acq(j);
  time_point<system_clock, nanoseconds> today =
      floor<days>(system_clock::now());

  cout << "size: " << acq.size() << endl << "capa: " << acq.capa() << endl;
  time_point<system_clock, nanoseconds> tp = system_clock::now();

  acq.fill_buffer();

  for (auto &v : acq.data()) {

    cout << fixed << setprecision(6) << v.time_since(today) << " " << v.data[0]
         << " " << v.data[1] << " " << v.data[2] << endl;
  }

  return 0;
}