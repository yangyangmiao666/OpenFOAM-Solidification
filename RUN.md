# OpenFOAM-Solidification — 在 Multipass `openfoam` 上运行（OF14）

本仓库已迁到 **OpenFOAM 14**。算例在虚拟机：

`/home/ubuntu/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification`

本机对应目录：

`/Users/yangyangmiao/myy/github_projects/OpenFOAM-Solidification`

---

## 1. 进入虚拟机并加载 OpenFOAM

```bash
multipass shell openfoam
# 若用 zsh（默认），OpenFOAM 已在 ~/.zshrc；否则：
source /opt/openfoam14/etc/bashrc
```

确认求解器：

```bash
which solidificationFoam
# 期望: .../platforms/linuxArm64GccDPInt32Opt/bin/solidificationFoam
```

---

## 2. 编译自定义求解器（首次或改过源码后）

```bash
cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/applications/solvers/solidificationFoam
./Allwmake -j
```

清理重编：`./Allwclean && ./Allwmake -j`

---

## 3. 运行 SMACS_I（串行试跑）

```bash
cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I
./Allclean
./Allrun
# 等价于: blockMesh && solidificationFoam
```

日志：`log.blockMesh`、`log.solidificationFoam`。成功结尾有 `End`，结果目录如 `0.05/`。

### 并行计算（指定核数）

算例目录（虚拟机内先 `cd` 到这里）：

```bash
cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I
```

虚拟机有 **10** 个 vCPU，建议 4–8 核（网格约 4.8 万单元）。

**推荐：一条命令指定核数**

```bash
./Allclean
./Allrun-parallel 8
# 等价: NP=8 ./Allrun-parallel
```

`Allrun-parallel` 会：写入 `numberOfSubdomains` → `blockMesh` → 清旧 `processor*` / `log.solidificationFoam` → `decomposePar` → `mpirun -np N solidificationFoam -parallel` → `reconstructPar`。

**只改字典再跑**

编辑 `system/decomposeParDict`：

```cpp
numberOfSubdomains 8;
method          scotch;
```

```bash
./Allclean
./Allrun-parallel
```

**逐步手写（与脚本等价）**

```bash
blockMesh
foamDictionary -entry numberOfSubdomains -set 8 system/decomposeParDict
rm -rf processor* log.solidificationFoam
decomposePar
mpirun -np 8 solidificationFoam -parallel
reconstructPar
```

注意：`numberOfSubdomains` 必须与 `mpirun -np` 一致；改核数后要重新分区。

**本机 Mac 上直接开并行（不必先 shell 进 VM）**

```bash
multipass exec openfoam -- bash -lc \
  'source /opt/openfoam14/etc/bashrc && \
   cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I && \
   ./Allclean && ./Allrun-parallel 8'
```

---

### 查看日志与进度（命令清单）

以下均在 **SMACS_I** 算例目录执行；另开一个终端跟进度最方便。

**实时跟求解进度（最常用）**

```bash
cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I
tail -f log.solidificationFoam
# 退出跟看: Ctrl+C
```

**看已写完的日志末尾（是否 `End` / `Finalising parallel run`）**

```bash
tail -n 80 log.solidificationFoam
tail -n 40 log.decomposePar
tail -n 40 log.reconstructPar
tail -n 40 log.blockMesh
```

**看有哪些结果时间目录 / 分片**

```bash
ls -ld [0-9]* processor* 2>/dev/null
ls processor0          # 并行中间时间步，如 0  0.05
ls 0.05                # 重建后的场: T C U alpha.solid p_rgh …
ls postProcessing/residuals 2>/dev/null
```

**进程是否还在跑**

```bash
pgrep -af solidificationFoam || echo 'no solidificationFoam process'
```

**本机 Mac 上远程看日志（不必进 shell）**

```bash
# 实时
multipass exec openfoam -- bash -lc \
  'tail -f ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I/log.solidificationFoam'

# 末尾快照 + 目录一览
multipass exec openfoam -- bash -lc '
CASE=~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I
cd "$CASE"
ls -lt log.*
ls -ld [0-9]* processor* 2>/dev/null
pgrep -af solidificationFoam || echo no_solver
tail -n 50 log.solidificationFoam
'
```

| 看什么 | 命令 / 路径 |
|--------|-------------|
| 求解进度 | `tail -f log.solidificationFoam` |
| 网格 / 分区 / 重建 | `log.blockMesh`、`log.decomposePar`、`log.reconstructPar` |
| 重建后的场 | 时间目录 `0.05/`、`1/`… |
| 并行中间结果 | `processor0/` … `processorN/` |
| 残差函数 | `postProcessing/residuals/` |
| 算完标志 | 日志末尾 `End`；并行还有 `Finalising parallel run` |

重要：若出现 `solidificationFoam already run`，说明旧 `log.solidificationFoam` 还在，求解被跳过。请先 `./Allclean`，或用当前的 `./Allrun-parallel N`（脚本会清掉旧 solver log 和 `processor*`）。

可视化：

```bash
paraFoam -case .
```

### 从本机一条命令跑

```bash
multipass exec openfoam -- bash -lc \
  'source /opt/openfoam14/etc/bashrc && \
   cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I && \
   ./Allclean && ./Allrun'
```

本机并行（例如 8 核）：

```bash
multipass exec openfoam -- bash -lc \
  'source /opt/openfoam14/etc/bashrc && \
   cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I && \
   ./Allclean && ./Allrun-parallel 8'
```

---

## 4. 怎么改参数（主要旋钮）

| 文件 | 控制什么 |
|------|----------|
| `system/controlDict` | 时间：`endTime`、`deltaT`、`writeInterval`；是否自适应步长 |
| `constant/thermoProperties` | 合金相图：熔点/共晶、浓度、潜热 |
| `constant/transportProperties` | 固/液物性：密度、Cp、导热、粘度、扩散、浮力系数 |
| `system/blockMeshDict` | 几何尺寸与网格密度 `(nx ny nz)` |
| `system/fvSolution` | 线性求解器容差、PIMPLE / 热迭代次数 |
| `system/fvSchemes` | 离散格式 |
| `0/T` | 初始温度与边界（右侧 Robin 冷却） |
| `0/C` | 初始浓度与边界 |
| `0/U`、`0/p_rgh`、`0/alpha.solid` | 速度、压力、固相分数初值/边界 |
| `system/decomposeParDict` | 并行分区数 |

### 时间（`system/controlDict`）

当前为**试跑**：`endTime 0.05`，`deltaT 0.005`，`adjustTimeStep no`。

正式物理目标约 **600 s**。固定 `deltaT=0.005` 时 Courant 会很快飙高（试跑末 `maxCo ~ 78`）。生产建议：

```cpp
endTime         600;
deltaT          1e-4;          // 先改小，或开自适应
writeInterval   1;             // 按需求
adjustTimeStep  yes;
maxCo           0.5;           // 若求解器/controlDict 支持；否则保持更小固定 deltaT
```

改完后重新 `./Allrun`（或从最近时间步 `startFrom latestTime` 续算）。

### 合金热力学（`constant/thermoProperties`）

- `Tmelt` / `Teut`：熔点、共晶温度 [K]
- `Calpha` / `Ceut`：相图浓度端点
- `L`：凝固潜热
- `Tiso`：等温区间相关参数

### 输运 / 浮力（`constant/transportProperties`）

- `solid` / `liquid` 下的 `rho`、`Cp`、`kappa`、`mu`、`D`
- 液体：`betaT`、`betaC`、`TRef`、`CRef`（热溶质浮力）
- `DAS`：枝晶臂间距（渗透率模型）

### 网格（`system/blockMeshDict`）

- `convertToMeters 0.01` → 顶点坐标单位为 cm
- 域约 5×6×0.1 cm；块 `(200 240 1)` ≈ 4.8 万单元
- 加密：增大 `(200 240 1)` 后必须重新 `blockMesh`（`./Allclean && ./Allrun`）

### 边界条件

- **`0/T`**：初温 `558.638 K`；`right` 为 `robinTemperature`（`h=400`，`Tinf=298`）— 改 `h`/`Tinf` 即改冷却强度
- **`0/C`**：初浓度 `0.18`；壁面 `zeroGradient`
- 左边界 `symmetryPlane`；`frontAndBack` 在 mesh 里是 `empty`（2D）

### 求解器控制（`system/fvSolution`）

- `PIMPLE.nThermoCorr`：热/相变内迭代（默认 10）
- `solvers` 里 `T|C|U`、`p_rgh` 的 `tolerance` / `relTol`

---

## 5. 后处理（ParaView / Tecplot）

算例目录（VM）：

`~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I`

并行算完后先重建成完整场（否则顶层没有 `0.05/`、`1/`…，只有 `processor*`）：

```bash
cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I
# 若求解还在跑，等日志出现 End / Finalising parallel run 再重建
reconstructPar
# 只要最新一步: reconstructPar -latestTime
```

本机已装 **ParaView 6.2**（`/Applications/ParaView-6.2.0.app`）。虚拟机里虽有 `paraview`/`paraFoam`，但无图形界面，**推荐把结果拿到本机用 ParaView**；Tecplot 同理。

### 5.1 ParaView（推荐）：直接读 OpenFOAM 算例

1. 在 VM 里重建后，打包**不含** `processor*` 的算例（体积小很多）：

```bash
multipass exec openfoam -- bash -lc \
  'cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam && \
   tar --exclude="SMACS_I/processor*" --exclude="SMACS_I/log.*" \
       -czf /tmp/SMACS_I-post.tar.gz SMACS_I'
multipass transfer openfoam:/tmp/SMACS_I-post.tar.gz /tmp/SMACS_I-post.tar.gz
mkdir -p /tmp/SMACS_I-post && tar -xzf /tmp/SMACS_I-post.tar.gz -C /tmp/SMACS_I-post
```

2. 建空的 `.foam` 标记文件并打开：

```bash
CASE=/tmp/SMACS_I-post/SMACS_I
touch "$CASE/SMACS_I.foam"
open -a ParaView "$CASE/SMACS_I.foam"
# 或: /Applications/ParaView-6.2.0.app/Contents/MacOS/paraview "$CASE/SMACS_I.foam"
```

3. 在 ParaView 里点 **Apply**，勾选 `T`、`C`、`U`、`alpha.solid` 等；时间轴可切换写出时刻。

并行未重建时，也可在 ParaView 的 OpenFOAM reader 里勾选 **Decomposed Case** 直接读 `processor*`（需把整个含 `processor*` 的目录拷回，体积很大）。

虚拟机无头模式（一般不推荐）：

```bash
# 仅当配置了 X11/VNC 时
paraFoam -case .
```

### 5.2 转成 VTK 再给 ParaView / Tecplot

在 **VM、已 `reconstructPar` 的算例**上：

```bash
source /opt/openfoam14/etc/bashrc
cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I
foamToVTK -latestTime
# 或全部时间步: foamToVTK
# 只要部分场: foamToVTK -fields '(T C U alpha.solid)'
```

会生成 `VTK/`。拷回本机：

```bash
multipass exec openfoam -- bash -lc \
  'cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I && \
   tar -czf /tmp/SMACS_I-VTK.tar.gz VTK'
multipass transfer openfoam:/tmp/SMACS_I-VTK.tar.gz /tmp/SMACS_I-VTK.tar.gz
```

- **ParaView**：File → Open → 选 `VTK/*.vtk` 或系列 `.vtm`/`.vtp`
- **Tecplot 360**：File → Load Data File(s) → 选 VTK / 或先转 EnSight（见下）

### 5.3 Tecplot

本仓库所用 OpenFOAM 14 **没有** `foamToTecplot360`。常用两条路：

1. **VTK**：上面 `foamToVTK` 后，Tecplot 用 VTK 加载器打开（视你的 Tecplot 版本而定）。
2. **EnSight**（Tecplot 支持较好）：

```bash
source /opt/openfoam14/etc/bashrc
cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I
foamToEnsight -latestTime
# 生成 EnSight 文件后打包拷回本机，Tecplot: Load → EnSight
```

若你的 Tecplot 带 **OpenFOAM loader**，也可直接指向重建后的算例目录（与 ParaView 的 `.foam` 类似，按 Tecplot 文档选 OpenFOAM 数据格式）。

### 5.4 结果在哪

| 内容 | 位置 |
|------|------|
| 重建后的场 | 算例下时间目录 `0.05/`、`1/`… |
| 并行中间结果 | `processor0/` … `processorN/` |
| VTK | 算例下 `VTK/`（`foamToVTK` 后） |
| EnSight | `foamToEnsight` 生成的目录/文件 |
| 残差 | `postProcessing/residuals/` |

## 6. 本机 ↔ 虚拟机互传（`multipass transfer`）

本机路径：`/Users/yangyangmiao/myy/github_projects/OpenFOAM-Solidification`  
虚拟机路径：`/home/ubuntu/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification`

语法（在 **Mac 终端**执行）：

```bash
multipass transfer <本地路径> openfoam:<虚拟机路径>     # 本机 → VM
multipass transfer openfoam:<虚拟机路径> <本地路径>     # VM → 本机
```

目录要传时，先打包成 tar，或逐个文件传（`multipass transfer` 传目录行为因版本而异，整仓建议用 tar）。

### 6.1 单文件：本机 → 虚拟机

```bash
# 例：把改过的 controlDict / RUN.md / Allrun-parallel 推进 VM
multipass transfer \
  /Users/yangyangmiao/myy/github_projects/OpenFOAM-Solidification/RUN.md \
  openfoam:/home/ubuntu/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/RUN.md

multipass transfer \
  /Users/yangyangmiao/myy/github_projects/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I/system/controlDict \
  openfoam:/home/ubuntu/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I/system/controlDict

multipass transfer \
  /Users/yangyangmiao/myy/github_projects/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I/Allrun-parallel \
  openfoam:/home/ubuntu/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I/Allrun-parallel

# 脚本传过去后记得可执行
multipass exec openfoam -- chmod +x \
  /home/ubuntu/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I/Allrun-parallel
```

### 6.2 单文件：虚拟机 → 本机

```bash
# 例：拉回 RUN.md、求解日志、某个结果场
multipass transfer \
  openfoam:/home/ubuntu/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/RUN.md \
  /Users/yangyangmiao/myy/github_projects/OpenFOAM-Solidification/RUN.md

multipass transfer \
  openfoam:/home/ubuntu/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I/log.solidificationFoam \
  /tmp/log.solidificationFoam

# 结果时间目录里的单个场（如 T）
multipass transfer \
  openfoam:/home/ubuntu/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I/0.05/T \
  /tmp/SMACS_I_0.05_T
```

### 6.3 整仓：本机 → 虚拟机（覆盖 VM 里的拷贝）

```bash
cd /Users/yangyangmiao/myy/github_projects
tar --exclude=.git --exclude='**/processor*' --exclude='log.*' \
  -czf /tmp/OpenFOAM-Solidification.tar.gz OpenFOAM-Solidification

multipass transfer /tmp/OpenFOAM-Solidification.tar.gz openfoam:/tmp/

multipass exec openfoam -- bash -lc \
  'cd ~/OpenFOAM/ubuntu-14/run && rm -rf OpenFOAM-Solidification && \
   tar -xzf /tmp/OpenFOAM-Solidification.tar.gz && \
   chmod +x OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I/Allrun*'
```

改过求解器源码后，在 VM 里再编译：

```bash
multipass exec openfoam -- bash -lc \
  'source /opt/openfoam14/etc/bashrc && \
   cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/applications/solvers/solidificationFoam && \
   ./Allwmake -j'
```

### 6.4 整仓或结果：虚拟机 → 本机

```bash
# 在 VM 内打包（可只打算例结果，体积更小）
multipass exec openfoam -- bash -lc \
  'cd ~/OpenFOAM/ubuntu-14/run && \
   tar --exclude=.git -czf /tmp/OpenFOAM-Solidification-from-vm.tar.gz OpenFOAM-Solidification'

multipass transfer \
  openfoam:/tmp/OpenFOAM-Solidification-from-vm.tar.gz \
  /tmp/OpenFOAM-Solidification-from-vm.tar.gz

# 解压到本机临时目录查看，或覆盖本机仓库（覆盖前请自行备份）
mkdir -p /tmp/of-from-vm && tar -xzf /tmp/OpenFOAM-Solidification-from-vm.tar.gz -C /tmp/of-from-vm
```

只拉回 SMACS_I 结果（含 `0.05/`、日志，不含巨大 `processor*` 时可自行加 exclude）：

```bash
multipass exec openfoam -- bash -lc \
  'cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam && \
   tar --exclude="SMACS_I/processor*" -czf /tmp/SMACS_I-results.tar.gz SMACS_I'

multipass transfer openfoam:/tmp/SMACS_I-results.tar.gz /tmp/SMACS_I-results.tar.gz
```

### 6.5 拷到本机 Downloads（给 ParaView 用）

目标目录：`/Users/yangyangmiao/Downloads/`

**A. 只要重建后的场（推荐，体积小，适合 ParaView）**

算还在跑时也可先重建**已写完的最新时间步**（不中断求解）：

```bash
multipass exec openfoam -- bash -lc \
  'source /opt/openfoam14/etc/bashrc && \
   cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I && \
   reconstructPar -latestTime'
```

打包（排除庞大的 `processor*` 和日志）并落到 Downloads：

```bash
multipass exec openfoam -- bash -lc \
  'cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam && \
   tar --exclude="SMACS_I/processor*" --exclude="SMACS_I/log.*" \
       -czf /tmp/SMACS_I-for-paraview.tar.gz SMACS_I'

multipass transfer openfoam:/tmp/SMACS_I-for-paraview.tar.gz \
  /Users/yangyangmiao/Downloads/SMACS_I-for-paraview.tar.gz

mkdir -p /Users/yangyangmiao/Downloads/SMACS_I-paraview
tar -xzf /Users/yangyangmiao/Downloads/SMACS_I-for-paraview.tar.gz \
  -C /Users/yangyangmiao/Downloads/SMACS_I-paraview

touch /Users/yangyangmiao/Downloads/SMACS_I-paraview/SMACS_I/SMACS_I.foam
open -a ParaView /Users/yangyangmiao/Downloads/SMACS_I-paraview/SMACS_I/SMACS_I.foam
```

**B. 整包含并行分片（很大，当前可达数 GB）**

```bash
multipass exec openfoam -- bash -lc \
  'cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam && \
   tar -czf /tmp/SMACS_I-full.tar.gz SMACS_I'
multipass transfer openfoam:/tmp/SMACS_I-full.tar.gz \
  /Users/yangyangmiao/Downloads/SMACS_I-full.tar.gz
```

**C. 只要 VTK**

```bash
multipass exec openfoam -- bash -lc \
  'source /opt/openfoam14/etc/bashrc && \
   cd ~/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I && \
   reconstructPar -latestTime && foamToVTK -latestTime && \
   tar -czf /tmp/SMACS_I-VTK.tar.gz VTK'
multipass transfer openfoam:/tmp/SMACS_I-VTK.tar.gz \
  /Users/yangyangmiao/Downloads/SMACS_I-VTK.tar.gz
```

### 6.6 快捷对照


| 方向 | 命令骨架 |
|------|----------|
| 本机 → VM | `multipass transfer /本地/文件 openfoam:/虚拟机/文件` |
| VM → 本机 | `multipass transfer openfoam:/虚拟机/文件 /本地/文件` |
| 在 VM 执行 | `multipass exec openfoam -- bash -lc '…'` |
| 进 VM 交互 | `multipass shell openfoam` |

## 7. 试跑验收（本次）

- 编译：`solidificationFoam` → `$FOAM_USER_APPBIN`
- 串行 `endTime 0.05`：约 10 步，残差收敛，日志 `End`，写出 `0.05/`
- 并行 8 核：`./Allrun-parallel 8` 成功，`Finalising parallel run` + `reconstructPar`，结果在 `0.05/` 与 `processor0`…`7`
- 注意：固定大步长下 `maxCo` 很大；长算前务必减小 `deltaT` 或开自适应
