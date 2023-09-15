########## 配置部分 ##########

# 根据实际情况修改以下值
avz_dir = r"C:\Games\Plants vs. Zombies\Tools\src\AsmVsZombies"  # AvZ安装目录
batch_count = 10  # 批次数量
source = r"pe-activate.cpp"  # 源码文件名
destination = r"dest"  # 输出文件夹

########## 配置部分结束 ##########

from concurrent.futures import ThreadPoolExecutor
import subprocess, threading, os


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
        print(f"编译完毕 {compiled_num}/{batch_count}.")


with ThreadPoolExecutor() as executor:
    for i in range(1, batch_count + 1):
        print(f"开始编译 {i}/{batch_count}...")
        dll_name = f"batch_{i}_of_{batch_count}.dll"
        cmd = rf'set "PATH={avz_dir}\MinGW\bin;%PATH%" && "{avz_dir}\MinGW\bin\g++" ".\{source}" -std=c++1z -I "{avz_dir}\inc" -lavz -lgdi32 -L "{avz_dir}\bin" -shared -DBATCH_COUNT={batch_count} -DBATCH_INDEX={i} -o ".\{destination}\{dll_name}"'
        executor.submit(run_command, cmd)

print("完毕.")
