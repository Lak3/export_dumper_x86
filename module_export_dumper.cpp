// module_export_dumper.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>

struct ExportInfo_t
{
	std::string m_szName{};

	DWORD m_dwIndex{};
	DWORD m_dwAddress{};
};

std::vector<ExportInfo_t> vecExports{};

void CacheExports(HANDLE hModule)
{
	const DWORD dwModule{ reinterpret_cast<DWORD>(hModule) };

	IMAGE_DOS_HEADER* pDosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(dwModule);
	IMAGE_NT_HEADERS32* pNtHeader = reinterpret_cast<IMAGE_NT_HEADERS32*>(dwModule + pDosHeader->e_lfanew);
	IMAGE_OPTIONAL_HEADER32* pOptHeader = &pNtHeader->OptionalHeader;
	IMAGE_DATA_DIRECTORY* pExpEntry = &pOptHeader->DataDirectory[0];
	IMAGE_EXPORT_DIRECTORY* pExpDir = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(dwModule + pExpEntry->VirtualAddress);

	WORD* pOrdTable = reinterpret_cast<WORD*>(dwModule + pExpDir->AddressOfNameOrdinals);
	DWORD* pAddressTable = reinterpret_cast<DWORD*>(dwModule + pExpDir->AddressOfFunctions);
	DWORD* pNameTable = reinterpret_cast<DWORD*>(dwModule + pExpDir->AddressOfNames);

	for (DWORD n{ 0x0 }; (n < pExpDir->NumberOfNames); n++)
	{
		char* pName = reinterpret_cast<char*>(dwModule + pNameTable[n]);

		/* I assume we can just skip it if it has no valid name ??? */
		if (!pName)
			continue;

		vecExports.push_back({ pName, n, pAddressTable[pOrdTable[n]] });
	}
}

int main()
{
	std::string szModuleName{}, szInputPenis{};

	std::cout << "Name of the module:\n";
	std::cin >> szModuleName;

	if (szModuleName.empty())
	{
		std::cout << "Module name not valid!\n";
		system("pause");
		return EXIT_FAILURE;
	}

	HANDLE hModule{ GetModuleHandleA(szModuleName.c_str()) };

	if (!hModule)
	{
		/* Still can't write LoadLibraryA without thinking of JX said back in the day */
		hModule = LoadLibraryA(szModuleName.c_str());

		if (!hModule)
		{
			std::cout << "Failed to access module!\n";
			system("pause");
			return EXIT_FAILURE;
		}
	}

	CacheExports(hModule);

	if (vecExports.empty())
	{
		std::cout << "No exports found from " << szModuleName.c_str() << "!\n";
		system("pause");
		return EXIT_FAILURE;
	}

	std::cout << "Cached " << vecExports.size() << " exports from " << szModuleName.c_str() << ", input P to print them here, S to save them to a file.\n";
	std::cin >> szInputPenis;

	if ((szInputPenis.front() == 'p') || (szInputPenis.front() == 'P'))
	{
		for (size_t n{}; (n < vecExports.size()); n++)
			std::cout << vecExports.at(n).m_szName.c_str() << ": index: 0x" << vecExports.at(n).m_dwIndex << ", offset: " << std::hex << "0x" << vecExports.at(n).m_dwAddress << "\n";
	}
	else if ((szInputPenis.front() == 's') || (szInputPenis.front() == 'S'))
	{
		/* This one gets the pass */
		std::string szSaveFile{ (std::string("./") + szModuleName + std::string("_exports.hpp")) };

		for (size_t n{}; (n < vecExports.size()); n++)
		{
			ExportInfo_t info{ vecExports.at(n) };

			/* This ones pretty stupid */
			std::string szKey{ std::to_string(info.m_dwIndex) + std::string(", ") + std::to_string(info.m_dwAddress) };

			WritePrivateProfileStringA("name_index_offset", info.m_szName.c_str(), szKey.c_str(), szSaveFile.c_str());
		}
	}
	else
	{
		std::cout << "Invalid input, idiot.\n";
	}

	std::cout << "Done.\n";
	system("pause");

	return EXIT_SUCCESS;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
