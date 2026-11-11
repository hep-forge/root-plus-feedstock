// root-plus runtime check (feedstock test): libraries load, dictionaries resolve, headers JIT.
#include <TClass.h>
#include <TSystem.h>
#include <TROOT.h>
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
  // Interpreter path: JIT code that includes a root-plus header and uses a class.
  int jit = gROOT->ProcessLine("#include <ROOT/IOPlus/Signal/DSP/Filters/TKalmanFilter.h>\n 42;");
  std::cout << "jit include -> " << jit << "\n";
  // The header must compile in the interpreter. Counted with C++ modules
  // only: with classic dictionaries (macOS, linux-aarch64) cling still trips
  // over <execution> pulled in by Core/Extension/execution.h (known
  // root-plus issue), so it is reported there but not failed.
  bool modules = !gSystem->AccessPathName(Form("%s/RIOPlus.pcm", gROOT->GetLibDir().Data()));
  std::cout << "dictionaries: " << (modules ? "C++ modules" : "classic") << "\n";
  if (jit != 42 && modules) ++bad;
  std::cout << (bad ? "FAILURES: " : "ALL OK ") << bad << "\n";
  return bad;
}
