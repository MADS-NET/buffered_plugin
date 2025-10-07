#include "acquisitor.hpp"
#include <iostream>

using namespace std;
using namespace std::chrono;
using json = nlohmann::json;

int main() {
  json j;
  j["capacity"] = 10;
  j["mean"] = 10;
  j["sd"] = 2;
  
  Acquisitor acq(j);
  time_point<system_clock, nanoseconds> today = floor<days>(system_clock::now());

  cout << "size: " << acq.size() << endl
       << "capa: " << acq.capa() << endl;

  acq.fill_buffer();

  for ( auto &v : acq.data()) {

    cout << fixed << setprecision(6) 
         << duration_cast<nanoseconds>(get<0>(v) - today).count() / 1.0E9 << " "
         << get<1>(v)[0] << " " 
         << get<1>(v)[1] << " " 
         << get<1>(v)[2] << endl;
  }
  
  return 0;
}