########## 配置部分 ##########

# 根据实际情况修改以下值
avz_dir = r"C:\Games\Plants vs. Zombies\Tools\src\AsmVsZombies"  # AvZ安装目录
dll_count = 10  # 编译为多少个dll文件
source = r"pe-activate.cpp"  # 源码文件名
destination = r"dest_test"  # 输出文件夹

########## 配置部分结束 ##########

from concurrent.futures import ThreadPoolExecutor
import subprocess, threading, os, json, sys

test_run = len(sys.argv) >= 2 and sys.argv[1] == "-test"
print(f"即将编译 {source} ({'试运行' if test_run else '正式运行'})")


if not os.path.exists(destination):
    os.makedirs(destination)

compiled_num = 0
lock = threading.Lock()


def run_command(cmd):
    global compiled_num
    process = subprocess.Popen(cmd, shell=True)
    process.communicate()
    with lock:
        compiled_num += 1
        print(f"编译完毕 {compiled_num}/{dll_count}.")


with ThreadPoolExecutor() as executor:
    for i in range(1, dll_count + 1):
        print(f"开始编译 {i}/{dll_count}...")
        dll_name = f"batch_{i}_of_{dll_count}.dll"
        cmd = rf'set "PATH={avz_dir}\MinGW\bin;%PATH%" && "{avz_dir}\MinGW\bin\g++" ".\{source}" -std=c++1z -I "{avz_dir}\inc" -lavz -lgdi32 -L "{avz_dir}\bin" -shared -DBATCH_COUNT={dll_count} -DBATCH_INDEX={i} {"-DTEST_RUN" if test_run else ""} -o ".\{destination}\{dll_name}"'
        executor.submit(run_command, cmd)

with open(rf".\{destination}\run_config.json", "w") as f:
    json.dump({"completed_count": 0, "batch_count": dll_count}, f)

print("完毕.")
print(f"输出文件夹: {destination}")
