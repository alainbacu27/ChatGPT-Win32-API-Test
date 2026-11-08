#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0602 // Windows 8 o posterior
#endif

#include <windows.h>
#include <roapi.h>
#include <cstdio>

// Punteros a las funciones exportadas por combase.dll.
using RoInitializeFn = HRESULT (WINAPI*)(RO_INIT_TYPE initType);
using RoGetApartmentIdentifierFn = HRESULT (WINAPI*)(UINT64* apartmentIdentifier);
using RoUninitializeFn = void (WINAPI*)();

int main()
{
    // Carga combase.dll desde el directorio System32.
    HMODULE combase = LoadLibraryExW(
        L"combase.dll",
        nullptr,
        LOAD_LIBRARY_SEARCH_SYSTEM32);

    if (combase == nullptr)
    {
        const DWORD error = GetLastError();
        std::printf("Error: no se pudo cargar combase.dll (Win32=%lu).\n", error);
        return 1;
    }

    // Obtiene las direcciones de las funciones necesarias.
    const auto pRoInitialize = reinterpret_cast<RoInitializeFn>(
        GetProcAddress(combase, "RoInitialize"));

    const auto pRoGetApartmentIdentifier = reinterpret_cast<RoGetApartmentIdentifierFn>(
        GetProcAddress(combase, "RoGetApartmentIdentifier"));

    const auto pRoUninitialize = reinterpret_cast<RoUninitializeFn>(
        GetProcAddress(combase, "RoUninitialize"));

    if (pRoInitialize == nullptr ||
        pRoGetApartmentIdentifier == nullptr ||
        pRoUninitialize == nullptr)
    {
        std::printf("Error: falta alguna exportacion requerida en combase.dll:\n");
        if (pRoInitialize == nullptr)
            std::printf("  - RoInitialize\n");
        if (pRoGetApartmentIdentifier == nullptr)
            std::printf("  - RoGetApartmentIdentifier\n");
        if (pRoUninitialize == nullptr)
            std::printf("  - RoUninitialize\n");
        FreeLibrary(combase);
        return 2;
    }

    // Inicializa Windows Runtime en el apartamento multithreaded (MTA).
    const HRESULT initResult = pRoInitialize(RO_INIT_MULTITHREADED);

    if (FAILED(initResult))
    {
        std::printf(
            "RoInitialize fallo. HRESULT=0x%08lX\n",
            static_cast<unsigned long>(initResult));
        FreeLibrary(combase);
        return 3;
    }

    // Consulta el identificador único del apartamento actual.
    UINT64 apartmentIdentifier = 0;
    const HRESULT result = pRoGetApartmentIdentifier(&apartmentIdentifier);

    if (SUCCEEDED(result))
    {
        std::printf("RoGetApartmentIdentifier se ejecuto correctamente.\n");
        std::printf(
            "Identificador del apartamento: 0x%016llX\n",
            static_cast<unsigned long long>(apartmentIdentifier));
    }
    else
    {
        std::printf(
            "RoGetApartmentIdentifier fallo. HRESULT=0x%08lX\n",
            static_cast<unsigned long>(result));
    }

    // Cada inicialización correcta debe equilibrarse con RoUninitialize.
    pRoUninitialize();
    FreeLibrary(combase);

    return SUCCEEDED(result) ? 0 : 4;
}
