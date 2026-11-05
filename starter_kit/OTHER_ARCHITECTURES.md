# Other Operating System Architectures

## 1. Architectures and Examples

**Microkernel – MINIX 3:** A microkernel keeps only essential functions, such as IPC and basic scheduling, inside the kernel. Services such as file systems and device drivers run as separate user-space servers. Its main advantage over a monolithic kernel is better fault isolation and reliability. Its disadvantage is additional IPC and context-switch overhead.

**Hybrid Kernel – Windows NT:** A hybrid kernel combines ideas from microkernels and monolithic kernels. It keeps many important services and drivers in kernel space to achieve good performance while maintaining some structural separation. Its advantage is balancing performance and modularity; its disadvantage is a large and complex privileged kernel with less fault isolation than a pure microkernel.

**Exokernel – MIT Aegis (research system):** An exokernel securely manages and allocates hardware resources but provides very few high-level abstractions. Applications use library operating systems to implement the abstractions they need. Its advantage is flexibility and application-specific resource management; its disadvantage is that applications become more complex to develop and maintain.

**Unikernel – MirageOS:** A unikernel combines an application with only the operating-system components it needs into one specialized image. It usually runs directly on a hypervisor. Its advantages include a small memory footprint and reduced attack surface; its disadvantage is limited flexibility for general-purpose workloads and more difficult debugging.

## 2. Read Operation in a Microkernel

Assume that the file system and disk driver are separate user-space servers:

1. The `copy_file` application calls `read()` through `OSApi`.
2. The request enters the microkernel.
3. The microkernel sends an IPC message to the file-system server.
4. The file-system server checks the handle and requests disk data from the disk-driver server through another IPC message.
5. The disk driver reads the required block and sends the data back to the file-system server.
6. The file-system server extracts the requested bytes and returns them to the application through the microkernel.

**Domain switches:** They are expected to increase compared with the monolithic architecture. A monolithic system normally has one user-to-kernel entry and one return per API call (two switches). A simplified microkernel path may involve eight domain transitions: application → kernel → file-system server → kernel → disk-driver server → kernel → file-system server → kernel → application.

**Data copies:** They are also expected to increase because data and messages may need to move between separate address spaces. Shared memory can reduce data copying, but IPC and coordination are still required.

## 3. Classification of Linux, Windows, and macOS

Linux is a modular monolithic kernel because most core services run in kernel space, while loadable modules provide runtime extensibility. Windows NT and macOS (XNU) are generally classified as hybrid kernels because they combine ideas from different architectures. None is a completely pure model because real operating systems combine architectural ideas to balance performance, reliability, compatibility, and extensibility.
