template <typename T>
struct MarkedPtr {
    T* ptr;
    unsigned int tag;
    bool marked = false;

    MarkedPtr(): ptr(nullptr), tag(1) {}
    explicit MarkedPtr(T* p): ptr(p), tag(1) {}
    explicit MarkedPtr(bool mark) : ptr(nullptr), marked(mark), tag(1) {}
    MarkedPtr(T* p, unsigned int n): ptr(p), tag(n) {}
    MarkedPtr(T* p, bool mark) : ptr(p), marked(mark), tag(1) {}
    MarkedPtr(T*p, bool mark, unsigned int n) : ptr(p), marked(mark), tag(n) {}

    T* operator->() const {
        return ptr;
    }
};