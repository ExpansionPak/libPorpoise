#include "gx.hpp"

extern "C" {

void GXDestroyTexObj(GXTexObj* obj_) {
  auto* obj = reinterpret_cast<GXTexObj_*>(obj_);
  if (obj->ref) {
    delete obj->ref;
    obj->ref = nullptr;
  }
}

void GXDestroyTlutObj(GXTlutObj* obj_) {
  auto* obj = reinterpret_cast<GXTlutObj_*>(obj_);
  if (obj->ref) {
    delete obj->ref;
    obj->ref = nullptr;
  }
}
}