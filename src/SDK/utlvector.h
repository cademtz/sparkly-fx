/**
 * Purpose: A hack header to stub CUtlVector. It will never be used by this client and requires way too much junk to add.
 */
#pragma once

template <class T>
class CUtlVector {
    CUtlVector() = delete;
    CUtlVector(const CUtlVector<T>&) = delete;
};
