from enum import Enum


class SEMode(Enum):
    MANUAL = -1  # 不自动进入SE，改为手动进入
    DAY = 0
    NIGHT = 1
    POOL = 2
    FOG = 3
    ROOF = 4


########## 配置部分 ##########

# 根据实际情况修改以下值
pvz_shortcut = r"bin\pvz"  # PvZ快捷方式路径
injector = r"bin\injector.exe"  # 注入器路径
dll_folder = r"dest"  # dll文件夹
batch_count = 10  # 批次数量
batch_to_run = (1, 2)  # 要运行的批次（含）
se_mode = SEMode.POOL  # 存档所在的SE模式（需启用隐藏页面）
loading_time_seconds = 5  # PvZ从启动到加载完毕所需时间，若配置较低请增加此值

########## 配置部分结束 ##########

import subprocess, threading, os, time

current_directory = os.getcwd()
lock = threading.Lock()
batch_start, batch_end = batch_to_run

for i in range(batch_end - batch_start + 1):
    os.startfile(rf".\{pvz_shortcut}")
time.sleep(1.5)

print("开始注入...")
all_dll_names = ""
for i in range(batch_start, batch_end + 1):
    all_dll_names += (
        rf'"{current_directory}\{dll_folder}\batch_{i}_of_{batch_count}.dll" '
    )
process = subprocess.Popen(rf".\{injector} {all_dll_names}")
process.wait()
print("注入完毕.")

import ctypes
import time
from ctypes.wintypes import HDC, INT, HWND, UINT, WPARAM, LPARAM, BOOL

# 常量
WM_LBUTTONDOWN = 0x0201
WM_LBUTTONUP = 0x0202
MK_LBUTTON = 0x0001
SW_RESTORE = 9
HORZRES = 8
DESKTOPHORZRES = 118
MOUSE_POS = {
    SEMode.DAY: [(550, 385), (447, 597), (88, 510), (290, 379)],
    SEMode.NIGHT: [(550, 385), (447, 598), (243, 510), (290, 379)],
    SEMode.POOL: [(550, 385), (398, 475), (290, 379)],
    SEMode.FOG: [(550, 385), (447, 599), (398, 510), (290, 379)],
    SEMode.ROOF: [(550, 385), (447, 600), (553, 510), (290, 379)],
}

# 变量
dpi_scale = 1.0
pvz_windows = []

# win32 动态库
user32 = ctypes.windll.user32
gdi32 = ctypes.windll.gdi32

# win32 函数接口 和 参数常量
GetDC = user32.GetDC
GetDC.argtypes = [HWND]
GetDC.restype = HDC

GetDeviceCaps = gdi32.GetDeviceCaps
GetDeviceCaps.argtypes = [HDC, INT]
GetDeviceCaps.restype = INT

ReleaseDC = user32.ReleaseDC
ReleaseDC.argtypes = [HWND, HDC]
ReleaseDC.restype = INT

PostMessageW = user32.PostMessageW
PostMessageW.argtypes = [HWND, UINT, WPARAM, LPARAM]
PostMessageW.restype = BOOL

EnumWindowsProc = ctypes.WINFUNCTYPE(ctypes.c_int, ctypes.c_void_p, ctypes.c_void_p)


def enum_windows_callback(hwnd, lparam):
    max_length = 256
    class_name = ctypes.create_string_buffer(max_length)
    title = ctypes.create_string_buffer(max_length)
    user32.GetClassNameA(hwnd, class_name, max_length)
    user32.GetWindowTextA(hwnd, title, max_length)
    if b"MainWindow" in class_name.value and b"Plants vs. Zombies" in title.value:
        pvz_windows.append(hwnd)
    return 1


def get_dpi_scale():
    screen = GetDC(None)
    if screen is not None:
        virtual_width = GetDeviceCaps(screen, HORZRES)
        physical_width = GetDeviceCaps(screen, DESKTOPHORZRES)
        ReleaseDC(None, screen)
        scale = physical_width / virtual_width
    else:
        scale = 1.0
    global dpi_scale
    dpi_scale = scale


def MAKELONG(low, high):
    if dpi_scale != 1.0:
        low, high = int(low / dpi_scale), int(high / dpi_scale)
    else:
        low, high = int(low), int(high)
    return ((high & 0xFFFF) << 16) | (low & 0xFFFF)


def left_click(hwnd, x, y):
    coord = MAKELONG(x, y)
    PostMessageW(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, coord)
    PostMessageW(hwnd, WM_LBUTTONUP, MK_LBUTTON, coord)


# 自动进入SE模式
get_dpi_scale()
enum_windows = EnumWindowsProc(enum_windows_callback)
user32.EnumWindows(enum_windows, 0)

print(f"找到{len(pvz_windows)}个pvz窗口.")
if len(pvz_windows) < batch_end - batch_start + 1:
    print(f"没有开启足够的pvz窗口, 程序中止.")


if se_mode in MOUSE_POS:
    time.sleep(loading_time_seconds)
    print("开始点击...")
    for hwnd in pvz_windows:
        left_click(hwnd, 400, 400)
    time.sleep(1.5)
    for x, y in MOUSE_POS[se_mode]:
        for hwnd in pvz_windows:
            left_click(hwnd, x, y)
        time.sleep(0.25)

print("完毕.")
