#include "std_hashes.hpp"

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#else
#include <openssl/evp.h>
#endif

namespace stdhash {

const char* name(Kind kind) {
    switch (kind) {
        case Kind::MD5: return "MD5";
        case Kind::SHA1: return "SHA-1";
        default: return "SHA-256";
    }
}

std::size_t digest_bytes(Kind kind) {
    switch (kind) {
        case Kind::MD5: return 16;
        case Kind::SHA1: return 20;
        default: return 32;
    }
}

#ifdef _WIN32

namespace {
// Algoritmo "rankena" atidaroma vieną kartą kiekvienai rūšiai ir laikoma
// programos gyvavimo laiką – kitaip kiekvienas kvietimas mokėtų už atidarymą
// ir spartos palyginimas būtų nesąžiningas.
BCRYPT_ALG_HANDLE handle_for(Kind kind) {
    static BCRYPT_ALG_HANDLE handles[3] = {nullptr, nullptr, nullptr};
    const int idx = static_cast<int>(kind);
    if (handles[idx] == nullptr) {
        LPCWSTR id = kind == Kind::MD5 ? BCRYPT_MD5_ALGORITHM
                   : kind == Kind::SHA1 ? BCRYPT_SHA1_ALGORITHM
                                        : BCRYPT_SHA256_ALGORITHM;
        if (BCryptOpenAlgorithmProvider(&handles[idx], id, nullptr, 0) != 0)
            handles[idx] = nullptr;
    }
    return handles[idx];
}
}  // namespace

std::vector<std::uint8_t> digest(Kind kind, const std::uint8_t* data, std::size_t size) {
    std::vector<std::uint8_t> out(digest_bytes(kind));
    BCRYPT_ALG_HANDLE alg = handle_for(kind);
    if (alg == nullptr) return {};
    BCRYPT_HASH_HANDLE h = nullptr;
    if (BCryptCreateHash(alg, &h, nullptr, 0, nullptr, 0, 0) != 0) return {};
    bool ok = BCryptHashData(h, const_cast<PUCHAR>(data), static_cast<ULONG>(size), 0) == 0 &&
              BCryptFinishHash(h, out.data(), static_cast<ULONG>(out.size()), 0) == 0;
    BCryptDestroyHash(h);
    return ok ? out : std::vector<std::uint8_t>{};
}

#else

std::vector<std::uint8_t> digest(Kind kind, const std::uint8_t* data, std::size_t size) {
    const EVP_MD* md = kind == Kind::MD5 ? EVP_md5() : kind == Kind::SHA1 ? EVP_sha1() : EVP_sha256();
    std::vector<std::uint8_t> out(EVP_MAX_MD_SIZE);
    unsigned int len = 0;
    if (EVP_Digest(data, size, out.data(), &len, md, nullptr) != 1) return {};
    out.resize(len);
    return out;
}

#endif

}  // namespace stdhash
