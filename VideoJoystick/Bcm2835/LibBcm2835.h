//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#ifndef LIBBCM2835_H
#define LIBBCM2835_H

#include <memory>

/// <summary>
/// libbcm2835 lifetime support; get a shared instance to assure that
/// the library is initialized, library will deinitialize when all
/// shared instances go out of scope
/// </summary>
class LibBcm2835 final {
public:
   ~LibBcm2835();

   static std::shared_ptr<LibBcm2835> Initialize();

private:
   LibBcm2835();
};

#endif // BCM2835_H
