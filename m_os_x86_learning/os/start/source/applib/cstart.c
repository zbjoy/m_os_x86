
int main(int argc, char** argv);
    // 这里是用户态的入口函数, 需要设置用户态的页目录表
    // mmu_set_page_dir(0); // 设置页目录表为0, 让它使用内核的页目录表

void cstart(int argc, char** argv) {
    main(argc, argv); // 调用用户态的入口函数
}
