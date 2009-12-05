/*
 * Copyright 2008, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef NATIVE_CLIENT_TESTS_NPAPI_PI_BASE_OBJECT_H_
#define NATIVE_CLIENT_TESTS_NPAPI_PI_BASE_OBJECT_H_

#include <nacl/nacl_npapi.h>

// Helper class that maps calls to the NPObject into virtual methods.
class BaseObject : public NPObject {
 public:
  virtual ~BaseObject() {
  }

  virtual void Invalidate() {
  }

  virtual bool HasMethod(NPIdentifier name) {
    return false;
  }

  virtual bool Invoke(NPIdentifier name,
                      const NPVariant* args, uint32_t arg_count,
                      NPVariant* result) {
    return false;
  }

  virtual bool InvokeDefault(const NPVariant* args, uint32_t arg_count,
                             NPVariant* result) {
    return false;
  }

  virtual bool HasProperty(NPIdentifier name) {
    return false;
  }

  virtual bool GetProperty(NPIdentifier name, NPVariant* result) {
    return false;
  }

  virtual bool SetProperty(NPIdentifier name, const NPVariant* value) {
    return false;
  }

  virtual bool RemoveProperty(NPIdentifier name) {
    return false;
  }
};

#endif  // NATIVE_CLIENT_TESTS_NPAPI_PI_BASE_OBJECT_H_