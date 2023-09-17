########## 配置部分 ##########

data_folder = "C:\Games\Plants vs. Zombies\Data\刷新\自然自测"  # 数据文件所在目录

########## 配置部分结束 ##########

import numpy as np
import pandas as pd
from sklearn.utils import resample
import warnings, os, time

start_time = time.time()

warnings.filterwarnings("ignore", category=UserWarning, module="openpyxl")

print(f"数据文件目录: {data_folder}")


def get_confidence_interval_using_bootstrap(data, REPEAT=2_000):
    # https://acclab.github.io/bootstrap-confidence-intervals.html
    data = np.array(data)
    bootstrap_means = []

    for _ in range(REPEAT):
        bootstrap_sample = resample(data, replace=True, n_samples=len(data))
        bootstrap_means.append(np.mean(bootstrap_sample))

    return np.percentile(bootstrap_means, [2.5, 97.5])


xlsx_file_paths = []

for foldername, subfolders, filenames in os.walk(data_folder):
    for filename in filenames:
        if filename.endswith(".xlsx"):
            file_path = os.path.join(foldername, filename)
            xlsx_file_paths.append(file_path)

print(f"共找到{len(xlsx_file_paths)}个.xlsx文件.")

with open("result.txt", "w", encoding="utf-8") as file:
    for i, file_path in enumerate(xlsx_file_paths):
        print(f"处理 {i + 1}/{len(xlsx_file_paths)}...")

        df = pd.read_excel(file_path, sheet_name="stats")
        average_accident_rate = float(df.columns[-1].strip("%"))

        accident_rates = []
        for hp_ratio in pd.read_excel(file_path, sheet_name="raw")["hp"]:
            hp_ratio = float(hp_ratio)
            accident_rate = 0
            if hp_ratio > 0.65:
                accident_rate = 1
            elif hp_ratio > 0.5:
                accident_rate = (hp_ratio - 0.5) / 0.15
            accident_rates.append(accident_rate * 100)

        confidence_interval = get_confidence_interval_using_bootstrap(accident_rates)

        file_name = file_path[(len(data_folder) + 1) :].rstrip(".xlsx")
        file.write(
            f"{file_name}: {average_accident_rate:.3f} ({confidence_interval[0]:.3f}~{confidence_interval[1]:.3f})\n"
        )

print(f"耗时 {time.time() - start_time:.3f}s.")
