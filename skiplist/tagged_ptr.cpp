template <typename T>
struct TaggedPtr {
    T* ptr ;
    unsigned int tag ;

    TaggedPtr(): ptr(nullptr), tag(1) {}
    explicit TaggedPtr(T* p): ptr(p), tag(1) {}
    TaggedPtr(T* p, unsigned int n): ptr(p), tag(n) {}
    explicit TaggedPtr(unsigned int n) : ptr(nullptr), tag(n) {}

    T* operator->() const {
        return ptr;
    }
};
