#pragma once
#include <string>
#include <random>
#include "../Network/Network.hpp"

namespace Utilities
{
    using NtCreateThreadExFn = NTSTATUS(NTAPI*)(PHANDLE hThread, ACCESS_MASK DesiredAccess, PVOID ObjectAttributes, HANDLE ProcessHandle, PVOID lpStartAddress, PVOID lpParameter, ULONG Flags, SIZE_T StackZeroBits, SIZE_T SizeOfStackCommit, SIZE_T SizeOfStackReserve, PVOID lpBytesBuffer);
    using NtSuspendProcessFn = NTSTATUS(NTAPI*)(HANDLE ProcessHandle);
    const NtCreateThreadExFn NtCreateThreadEx = reinterpret_cast<NtCreateThreadExFn>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtCreateThreadEx"));
    const NtSuspendProcessFn NtSuspendProcess = reinterpret_cast<NtSuspendProcessFn>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtSuspendProcess"));

    std::string RandomString(const std::size_t Length);
    // Wrapper for aes encryption and hex encoding.
    std::tuple<std::string, std::string> EncryptMessage(const std::string& Plain);
    // Wrapper for aes encryption and hex encoding
    // but without the generation of a new iv.
    std::string EncryptMessage(const std::string& Plain, const std::string& AesIvStr);
    // Wrapper for aes decryption and hex decoding.
    std::string DecryptMessage(const std::string& Cipher, const std::string& EncodedIv);
    // Wrapper for getting the string of a public key.
    // Generates the public key based on the private key
    // fed and returns it as a string in the PEM format.
    std::string GetPublicKeyStr(RSA::PrivateKey& PrivateKey);
    // Close the process.
    inline void KillOwnProcess();
}