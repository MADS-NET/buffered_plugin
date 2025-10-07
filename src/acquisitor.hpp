/*
     _                   _     _ _             
    / \   ___ __ _ _   _(_)___(_) |_ ___  _ __ 
   / _ \ / __/ _` | | | | / __| | __/ _ \| '__|
  / ___ \ (_| (_| | |_| | \__ \ | || (_) | |   
 /_/   \_\___\__, |\__,_|_|___/_|\__\___/|_|   
                |_|                            
Base class for acquiring data from external sources.
The base class simply generates random data, for actual use you are 
supposed to make an application-specific child class.
Author: paolo.bosetti@unitn.it
Date: 2025-10-07
*/
#pragma once

#include <vector>
#include <array>
#include <map>
#include <iostream>
#include <nlohmann/json.hpp>
#include <random>
#include <chrono>
#include <tuple>
#include <thread>

#define DEFAULT_SIZE 100

using namespace std;
using namespace std::chrono;
using json = nlohmann::json;


class AcquisitorException : exception {
public:
  const char * what() { return "Acquisitor: buffer is full"; }
};


struct runif {
  runif(double m=0, double sd=1) {
    set(m, sd);
  }
  void set(double m, double sd) {
    normal_distribution<double>::param_type p{m, sd};
    _d.param(p);
  }
  double get() { return _d(_gen); }
private:
  random_device _rd{};
  mt19937 _gen{_rd()};
  normal_distribution<double> _d;
};


template <typename T = array<double, 3>>
class Acquisitor {
public:
  struct sample {
    time_point<system_clock, nanoseconds> time;
    T data;
  };

  Acquisitor(json settings, size_t capa = 0) : _settings(settings) {
    if (capa == 0) capa = _settings.value("capacity", DEFAULT_SIZE);
    _capa = capa;
    _data.reserve(_capa);
    double m = _settings.value("mean", 0);
    double sd = _settings.value("sd", 0);
    _rnd.set(m, sd);
    setup();
  }

  // Initialize connections
  virtual void setup() {

  }

  // Single acquisition
  virtual void acquire() {
    if (!is_same<array<double, 3>, T>::value) {
      throw runtime_error("Base class only supports data of type std::array<double, 3>; implement child class for different types");
    }
    if (is_full()) throw AcquisitorException();

    tuple<time_point<system_clock, nanoseconds>, T> pair(
      system_clock::now(),
      {_rnd.get(), _rnd.get(), _rnd.get()}
    );
    _data.push_back(pair);
    this_thread::sleep_for(microseconds(50));
  }

  // Fill the buffer by calling acquire() until the buffer is full
  void fill_buffer(bool reset = true) {
    if (reset) _data.clear();
    while (true) {
      try {
        acquire();
      } catch(AcquisitorException &e) {
        break;
      }
    }
  }

  auto &data() { return _data; }
  T operator[](size_t i) { return _data[i]; }
  size_t size() { return _data.size(); }
  size_t capa() { return _capa; }
  bool is_full() { return _data.size() == _capa; }
  void reset() { _data.clear(); }

private:
  json _settings;
  size_t _capa;
  vector<tuple<time_point<system_clock, nanoseconds>, T>> _data;
  runif _rnd;
};




