#include <Windows.h>
#include <roapi.h>
#include <objbase.h>
#include <iostream>
#include <iomanip>

// Funciones exportadas por combase.dll
using PFN_RoInitialize = HRESULT(WINAPI*)(RO_INIT_TYPE);
using PFN_RoUninitialize = void(WINAPI*)();
using PFN_RoGetApartmentIdentifier = HRESULT(WINAPI*)(UINT64*);

int wmain()
{
    std::wcout << L"====================================\n";
    std::wcout << L"       COMBASE.DLL API TEST\n";
    std::wcout << L"====================================\n\n";

    // Cargar combase.dll
    HMODULE hCombase = LoadLibraryW(L"combase.dll");

    if (!hCombase)
    {
        std::wcerr << L"[ERROR] No se pudo cargar combase.dll\n";
        std::wcerr << L"Error: " << GetLastError() << L"\n";
        return 1;
    }

    std::wcout << L"[OK] combase.dll cargada correctamente.\n";

    // Obtener direcciones de las funciones
    auto pRoInitialize =
        reinterpret_cast<PFN_RoInitialize>(
            GetProcAddress(hCombase, "RoInitialize"));

    auto pRoUninitialize =
        reinterpret_cast<PFN_RoUninitialize>(
            GetProcAddress(hCombase, "RoUninitialize"));

    auto pRoGetApartmentIdentifier =
        reinterpret_cast<PFN_RoGetApartmentIdentifier>(
            GetProcAddress(hCombase, "RoGetApartmentIdentifier"));

    std::wcout << L"\n--- Exportaciones ---\n";

    std::wcout << L"RoInitialize: "
        << (pRoInitialize ? L"FOUND" : L"NOT FOUND") << L"\n";

    std::wcout << L"RoUninitialize: "
        << (pRoUninitialize ? L"FOUND" : L"NOT FOUND") << L"\n";

    std::wcout << L"RoGetApartmentIdentifier: "
        << (pRoGetApartmentIdentifier ? L"FOUND" : L"NOT FOUND") << L"\n";

    if (!pRoInitialize || !pRoUninitialize ||
        !pRoGetApartmentIdentifier)
    {
        FreeLibrary(hCombase);
        return 2;
    }

    std::wcout << L"\n--- Inicializacion COM/WinRT ---\n";

    // Inicializar Windows Runtime en modo MTA
    HRESULT hr = pRoInitialize(RO_INIT_MULTITHREADED);

    std::wcout << L"RoInitialize HRESULT: 0x"
        << std::hex << std::uppercase
        << static_cast<unsigned long>(hr) << L"\n";

    if (FAILED(hr))
    {
        std::wcerr << L"[ERROR] RoInitialize fallo.\n";
        FreeLibrary(hCombase);
        return 3;
    }

    std::wcout << L"[OK] Inicializacion completada.\n";

    // Obtener identificador del apartamento
    UINT64 apartmentId = 0;

    hr = pRoGetApartmentIdentifier(&apartmentId);

    std::wcout << L"\n--- Apartment Identifier ---\n";

    std::wcout << L"HRESULT: 0x"
        << std::hex << std::uppercase
        << static_cast<unsigned long>(hr) << L"\n";

    if (SUCCEEDED(hr))
    {
        std::wcout << L"Apartment ID: "
            << std::dec << apartmentId << L"\n";

        std::wcout << L"[OK] RoGetApartmentIdentifier funciona.\n";
    }
    else
    {
        std::wcout << L"[ERROR] No se pudo obtener el identificador.\n";
    }

    // Liberar inicializacion
    pRoUninitialize();

    std::wcout << L"\n[OK] RoUninitialize ejecutado.\n";

    FreeLibrary(hCombase);

    std::wcout << L"[OK] combase.dll liberada.\n";
    std::wcout << L"\nPrueba finalizada.\n";

    return SUCCEEDED(hr) ? 0 : 4;
}