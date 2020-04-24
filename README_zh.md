

# `"emo"`编程语言（中文版）

![workflow](https://github.com/PsiACE/emo/workflows/ci_meson/badge.svg)
![quality](https://www.code-inspector.com/project/6258/status/svg)

## [英文版](./README.md)| [中文版](./README_zh.md)

> “emo”	情感能让我们更好地表达自己。

“emo” 编程语言致力于探索简单而富有表现力的方法，功能强大且易于使用。

_**WIP**_本项目仍处于个人发展和研究阶段，只接受提交优化和补丁，但欢迎就设计和功能进行讨论。

如果你熟悉任何类似c语言的编程语言，你将很容易地理解“emo”的语法规则。当然，“emo”也从其他更现代的编程语言中获得了一些灵感，以提高可读性和编程体验。

## 使用

如果你想尝试一下，请安装 `meson` 和 `ninja` 
使用流程：

**安装 `meson`和`ninja`**

```bash
# 通过python包管理安装
python3 -m pip install meson
python3 -m pip install ninja
# Windows，mac，linux 下载安装
https://github.com/mesonbuild/meson/releases
https://github.com/ninja-build/ninja/releases
```

```bash
#通过包管理器安装
# Mac 安装
HomeBrew: brew install ninja
MacPorts: port install ninja
# Linux 安装
Arch: pacman -S ninja
Debian/Ubuntu: apt-get install ninja-build
Fedora: dnf install ninja-build
Gentoo: emerge dev-util/ninja
Opensuse: zypper in ninja
Alpine: apk add ninja
```

**配置项目环境**

```bash
#克隆本项目
git clone git@github.com:PsiACE/emo.git # or https://github.com/psiace/emo.git
#进入项目文件夹
cd emo
#环境配置
meson setup build
meson configure build -Dc_args="-O3" 
meson install -C build
```

现在‘emo’被添加到您的系统中。请在终端中运行' emo '或查看文档。

## 联系

Chojan Shang - [@PsiACE](https://github.com/psiace) - <psiace@outlook.com>

项目链接: [https://github.com/psiace/emo](https://github.com/psiace/emo)

## 开源声明

This project is licensed under the terms of the [MIT license](./LICENSE).

本项目是根据[MIT license](./LICENSE)获得许可的。

## Credits

制作编程语言的手册。“emo”的许多代码都直接或间接地来自这里。

链接：http://www.craftingexplters.com