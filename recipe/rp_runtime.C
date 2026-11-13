// root-plus runtime check (feedstock test): libraries load, dictionaries resolve, headers JIT.
#include <TClass.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TInterpreter.h>
#include <iostream>
int rp_runtime() {
  int bad = 0;
  for (const char* lib : {"libRIOPlus", "libRIOPlus_Config", "libRIOPlus_Format",
                          "libRIOPlus_Signal", "libRIOPlus_Tensor", "libRIOPlus_Cloud"}) {
    int rc = gSystem->Load(lib);
    std::cout << "load " << lib << " -> " << rc << "\n";
    if (rc < 0) ++bad;
  }
  for (const char* n : {"TArchive", "TCurl", "TCloud", "TAmplitudeSpectrumD",
                        "ROOT::IOPlus::TBiquadFilter<ROOT::IOPlus::Plane::Z>",
                        "ROOT::IOPlus::TCascadeFilter<ROOT::IOPlus::Plane::S>",
                        "ROOT::IOPlus::TKalmanFilter",
                        "ROOT::IOPlus::TContinuousWaveletTransform<double>"}) {
    TClass* c = TClass::GetClass(n);
    bool ok = c && c->HasDictionary();
    std::cout << (ok ? "OK   " : "MISS ") << n << (c ? Form("  (%s, size %d)", c->GetName(), c->Size()) : "") << "\n";
    if (!ok) ++bad;
  }
  // Interpreter path: JIT code that includes a root-plus header and uses it.
  // Every platform: with classic dictionaries (macOS, linux-aarch64) cling
  // parses the headers text-wise, and on macOS only the libc headers in ROOT's
  // own prebuilt module are safe there (root-plus keeps the others out).
  bool modules = !gSystem->AccessPathName(Form("%s/RIOPlus.pcm", gROOT->GetLibDir().Data()));
  std::cout << "dictionaries: " << (modules ? "C++ modules" : "classic") << "\n";
  // The #include on its own: with an expression in the same input, cling
  // wraps both in a function and would parse the header inside it.
  int err = TInterpreter::kNoError;  // ProcessLine reports into an int
  gROOT->ProcessLine("#include <ROOT/IOPlus/Signal/Kalman/TKalmanFilter.h>", &err);
  long jit = (err == TInterpreter::kNoError) ? gROOT->ProcessLine("42;", &err) : 0;
  std::cout << "jit include -> " << jit << " (error code " << err << ")\n";
  if (err != TInterpreter::kNoError || jit != 42) ++bad;
  std::cout << (bad ? "FAILURES: " : "ALL OK ") << bad << "\n";
  return bad;
}
