# 刷新测试器（扩展版）v0.1.0

## 与原刷新修改器的区别？

修改测试编写逻辑，无需手动写AvZ代码，而是直接用封装好的 `Cob`, `FixedCard` 与 `SmartCard`.

这样做的好处是：
- 自动生成文件名，对大批量测试非常友好
- 自动推测需要选的卡

这样做的局限是：
- 暂时无法测定控制变量的情况

## 文件命名规则

输出文件采用法棍式命名，以 `I(100) PP(301, {1,5}-9), J(600, 3, 9), A(900, 红白(1,2→2, 5,6→5)-9) y(红白) n(车) g(0) RHDU.xlsx` 为例：
```c++
// 操作部分
I(100)                         // 于100生效冰
PP(301, {1,5}-9)               // 于301生效炸(1,9)与炸(5,9)的炮
J(600, 3, 9)                   // 于600生效(3,9)的辣椒
A(900, 红白(1,2→2, 5,6→5)-9)    // 于900生效樱桃，智能判断红眼+白眼数，若1,2路多放2路，若5,6路多放6路，列数固定为9

// 设置部分
y(红白)        // 出怪包含红眼、白眼
n(车)          // 出怪不包含冰车
g(0)          // 红眼数量限制为0（无限制则不标注）
R             // 测试激活（若测试分离则不标注）
H             // 测试大波（若测试普通波则不标注）
D             // 开启Dance Cheat（若不开启则不标注）
U             // 使用极限出怪（若使用自然出怪则不标注）
```

特别地，使用 `J` 表示辣椒，因其节省文件名长度且更清晰；使用 `W` 表示窝瓜，因为NTFS不区分大小写；`I`, `A`, `J`, `W` 以外的卡片表示为 `Cxx`，其中 `xx` 为该卡片的自然序号（豌豆射手为0，依次类推）。

## 如何使用

本框架只支持AvZ1.

### 安装插件

将 `avz\inc` 目录下的文件复制至 `你的AvZ安装目录\inc` 下。请根据 `information.txt` 下载所依赖的其它插件。

### 配置脚本目录

笔者建议单独建一个用于存放测试脚本的目录。

假设此目录名为 `refresh_tests`, 请构建如下结构：
```
...\refresh_tests
└─bin
   ├─pvz（快捷方式）
   └─injector.exe
├─compile.py
├─run.py
├─脚本1.cpp
├─脚本2.cpp
└─...
```

以上结构与本仓库是对应的（`avz\inc` 与 `template` 除外）。

### 开测！

`compile.py` 负责从一份 `cpp` 源码中编译多个 `dll`, `run.py` 负责开启指定数量的pvz窗口并注入对应数量的dll以开启测试。

所有流程均为自动化，玩家只需要运行 python 脚本，以及测试结束时关闭 pvz 窗口。

请先根据实际情况，填写`compile.py`与`run.py`文件中的配置部分。

然后依次执行:
```console
.\compile.py
.\run.py
```

### 注意事项

- `cpp`源码中指定了输出目录。请确保该目录存在，且该目录下有对应的子目录（如`\红白\激活`, `\红白\分离`, `\红\激活`等9个）。你可以直接使用本仓库提供的 template 文件夹，其已包括所需的子目录。
- 推荐在正式测试前，先将测试选卡数改为1并试运行，确认输出文件完整后再改为1000.
- 基于试运行，你可以预估测试总时长，计算合适的批次数量。
    - 例如，如果预估10开的情况下需测8小时，那么可以先编译出80个dll，然后每次运行10个。这样每次运行的时长不会超过1小时。

## FAQ
> 为什么不和原有的refresh插件合并？

扩展版与原插件不兼容，暂时没有合并的打算。

> 我只看到了 `pe-active.cpp` ？

是的。它涵盖了[自然出怪论 Part2](https://docs.qq.com/doc/DTlpGdUpmYkpxZEVZ) 中 `PE\...\激活` 的所有测试。

更多测试脚本待补。欢迎向本仓库贡献并提交 pull request.
