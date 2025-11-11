#pragma once

//
//  MGTools DLL import/export control
//
#ifdef MGTOOLS_EXPORTS
    // Building the DLL
#define MGTOOLS_API __declspec(dllexport)
#else
    // Using the DLL
#define MGTOOLS_API __declspec(dllimport)
#endif

// For template specializations or inline helpers that must be visible
#define MGTOOLS_TEMPLATE extern
