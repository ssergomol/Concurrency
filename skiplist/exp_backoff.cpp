class ExpBackoff {
    int const nStep;
    int const nThreshold;
    int nCurrent;

public:
    explicit ExpBackoff( int init=4, int step=2, int threshold=400 )
            : nStep(step), nThreshold(threshold), nCurrent(init) {}

    void operator()() {
        for (int k = 0; k < nCurrent; ++k) {
            __asm volatile ("nop");
        }

        nCurrent *= nStep;
        if ( nCurrent > nThreshold ) {
            nCurrent = nThreshold;
        }
    }

};