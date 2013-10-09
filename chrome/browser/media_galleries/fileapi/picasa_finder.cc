// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/media_galleries/fileapi/picasa_finder.h"

#if defined(OS_WIN)
#include <windows.h>
#endif

#include "base/base_paths.h"
#include "base/bind.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/strings/string16.h"
#include "chrome/browser/storage_monitor/storage_info.h"
#include "chrome/common/media_galleries/picasa_types.h"
#include "content/public/browser/browser_thread.h"

#if defined(OS_WIN)
#include "base/win/registry.h"
#endif

namespace picasa {

#if defined(OS_WIN)
const wchar_t kPicasaRegistryPath[] =
    L"Software\\Google\\Picasa\\Picasa2\\Preferences";
const wchar_t kPicasaRegistryAppDataKey[] = L"AppLocalDataPath";
#endif

namespace {

#if defined(OS_WIN)
base::FilePath GetCustomPicasaRoot() {
  base::win::RegKey key;
  if (key.Open(HKEY_CURRENT_USER, kPicasaRegistryPath, KEY_READ) !=
          ERROR_SUCCESS || !key.Valid()) {
    return base::FilePath();
  }

  string16 value;
  if (key.ReadValue(kPicasaRegistryAppDataKey, &value) != ERROR_SUCCESS)
    return base::FilePath();
  if (value.empty())
    return base::FilePath();

  return base::FilePath(value);
}

base::FilePath GetPicasaDatabasePathWin() {
  base::FilePath path = GetCustomPicasaRoot();
  if (path.empty() && !PathService::Get(base::DIR_LOCAL_APP_DATA, &path))
    return base::FilePath();

  return path.AppendASCII("Google").AppendASCII("Picasa2").AppendASCII(
        kPicasaDatabaseDirName);
}
#endif

#if defined (OS_MACOSX)
base::FilePath GetPicasaDatabasePathMac() {
  // TODO(tommycli): Support Picasa Mac's custom path.
  base::FilePath path;
  if (!PathService::Get(base::DIR_APP_DATA, &path))
    return base::FilePath();

  // On Mac, the database is in "Picasa3", not "Picasa2".
  return path.AppendASCII("Google").AppendASCII("Picasa3").AppendASCII(
        kPicasaDatabaseDirName);
}
#endif

// Returns path of Picasa's DB3 database directory. May only be called on
// threads that allow for disk IO, like the FILE thread or MediaTaskRunner.
base::FilePath FindPicasaDatabaseOnFileThread() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::FILE));
  base::FilePath path;
  #if defined(OS_WIN)
  path = GetPicasaDatabasePathWin();
  #elif defined(OS_MACOSX)
  path = GetPicasaDatabasePathMac();
  #endif

  // Verify actual existence
  if (!base::DirectoryExists(path))
    path.clear();

  return path;
}

void FinishOnOriginalThread(const DeviceIDCallback& callback,
                            const base::FilePath& database_path) {
  std::string device_id;
  if (!database_path.empty()) {
    device_id = StorageInfo::MakeDeviceId(StorageInfo::PICASA,
                                          database_path.AsUTF8Unsafe());
  }
  callback.Run(device_id);
}

}  // namespace

void FindPicasaDatabase(const DeviceIDCallback& callback) {
  content::BrowserThread::PostTaskAndReplyWithResult(
      content::BrowserThread::FILE,
      FROM_HERE,
      base::Bind(&FindPicasaDatabaseOnFileThread),
      base::Bind(&FinishOnOriginalThread, callback));
}

}  // namespace picasa
