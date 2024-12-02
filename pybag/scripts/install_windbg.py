"""
Copyright 2020-2024 Vector 35 Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Original version : https://raw.githubusercontent.com/Vector35/debugger/refs/heads/dev/core/adapters/dbgeng/install_windbg.py

"""

import urllib.request
import xml.dom.minidom
import zipfile
import tempfile
import os
import pathlib

# Prefer WINDBG_DIR environment variable
# Otherwise try to install inside pybag
install_target = os.getenv('WINDBG_DIR')
if install_target is None or not os.path.exists(install_target):
    script_dir = pathlib.Path(__file__).parent.resolve()
    pybag_dir = script_dir.parent.resolve()
    install_target = os.path.join(pybag_dir, 'windbg')


def check_install_ok(path):
    if not os.path.exists(os.path.join(path, 'amd64', 'dbgeng.dll')):
        return False

    if not os.path.exists(os.path.join(path, 'amd64', 'dbghelp.dll')):
        return False

    if not os.path.exists(os.path.join(path, 'amd64', 'dbgmodel.dll')):
        return False

    if not os.path.exists(os.path.join(path, 'amd64', 'dbgcore.dll')):
        return False

    if not os.path.exists(os.path.join(path, 'amd64', 'ttd', 'TTD.exe')):
        return False

    if not os.path.exists(os.path.join(path, 'amd64', 'ttd', 'TTDRecord.dll')):
        return False

    return True


def install():
    if check_install_ok(install_target):
        print("Install detected at: %s" % install_target)
        return

    ttd_url = 'https://aka.ms/windbg/download'
    print('Downloading appinstaller from: %s...' % ttd_url)
    try:
        local_file, _ = urllib.request.urlretrieve(ttd_url)
    except Exception as e:
        print('Failed to download appinstaller file from %s' % ttd_url)
        print(e)
        return
    print('Successfully downloaded appinstaller')

    xml_doc = xml.dom.minidom.parse(local_file)
    try:
        msix_url = xml_doc.getElementsByTagName('MainBundle')[0].attributes['Uri'].value
    except Exception as e:
        print('Failed to parse XML')
        print(e)
        return

    print('Downloading MSIX bundle from: %s...' % msix_url)
    try:
        msix_file, _ = urllib.request.urlretrieve(msix_url)
    except Exception as e:
        print('Failed to download MSIX bundle from %s' % msix_url)
        print(e)
        return
    print('Successfully downloaded MSIX bundle')

    zip_file = zipfile.ZipFile(msix_file)
    temp_dir = tempfile.mkdtemp()
    inner_msix = zip_file.extract('windbg_win-x64.msix', temp_dir)
    print('Extracted windbg_win-x64 to %s' % inner_msix)


    print('Installing to: %s' % install_target)

    inner_zip = zipfile.ZipFile(inner_msix)
    inner_zip.extractall(install_target)

    if check_install_ok(install_target):
        print('WinDbg/TTD installed to %s!' % install_target)
    else:
        print('The WinDbg/TTD installation appears to be successful, but important files are missing from %s, '
              'and the TTD recording may not work properly.' % install_target)
        return

    #x64dbgEngPath = os.path.join(install_target, 'amd64')
    #x86dbgEngPath = os.path.join(install_target, 'x86')

if __name__ == '__main__':
    install()