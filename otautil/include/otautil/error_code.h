/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _ERROR_CODE_H_
#define _ERROR_CODE_H_

enum ErrorCode : int {
  kNoError = -1,
  kLowBattery = 20,
  kZipVerificationFailure,
  kZipOpenFailure,
  kBootreasonInBlacklist,
  kPackageCompatibilityFailure,
  kScriptExecutionFailure,
  kMapFileFailure,
  kForkUpdateBinaryFailure,
  kUpdateBinaryCommandFailure,
  kMountFailure,
  kUnmountFailure,
  kMetadataParseFailure,
  KPipeCreateFailure,
  kRunUpdateBinaryFailure,
};

enum CauseCode : int {
  kNoCause = -1,
  kArgsParsingFailure = 100,
  kStashCreationFailure,
  kFileOpenFailure,
  kLseekFailure,
  kFreadFailure,
  kFwriteFailure,
  kFsyncFailure,
  kLibfecFailure,
  kFileGetPropFailure,
  kFileRenameFailure,
  kSymlinkFailure,
  kSetMetadataFailure,
  kTune2FsFailure,
  kRebootFailure,
  kPackageExtractFileFailure,
  kPatchApplicationFailure,
  kHashTreeComputationFailure,
  kEioFailure,
  kSdcardFailure,
  kVendorFailure = 200
};

enum UncryptErrorCode : int {
  kUncryptNoError = -1,
  kUncryptErrorPlaceholder = 50,
  kUncryptTimeoutError = 100,
  kUncryptFileRemoveError,
  kUncryptFileOpenError,
  kUncryptSocketOpenError,
  kUncryptSocketWriteError,
  kUncryptSocketListenError,
  kUncryptSocketAcceptError,
  kUncryptFstabReadError,
  kUncryptFileStatError,
  kUncryptBlockOpenError,
  kUncryptIoctlError,
  kUncryptReadError,
  kUncryptWriteError,
  kUncryptFileSyncError,
  kUncryptFileCloseError,
  kUncryptFileRenameError,
  kUncryptPackageMissingError,
  kUncryptRealpathFindError,
  kUncryptBlockDeviceFindError,
};

class ErrorMessage {
public:
  ErrorMessage():mError(static_cast<ErrorCode>(0)),
                 mCause(static_cast<CauseCode>(0)),
                 mUncryptError(static_cast<UncryptErrorCode>(0)){
  }
  // XXX(uncrypt)-XXXX(error)-XXX(cause)
  // uncrypt < 400, error < 9999, cause < 999
  unsigned long GenerateCode() {
    return (10000000 * mUncryptError + 1000 * mError + mCause);
  }
  void SetErrorCode(ErrorCode code){mError=code;}
  void SetCauseCode(CauseCode code){mCause=code;}
  void SetUncryptErrorCode(UncryptErrorCode code){mUncryptError=code;}
private:
  ErrorCode mError;
  CauseCode mCause;
  UncryptErrorCode mUncryptError;
};

#endif  // _ERROR_CODE_H_
