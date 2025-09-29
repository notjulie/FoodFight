
#ifndef VJCONFIG_H
#define VJCONFIG_H

#include <filesystem>
#include "SQLDB.h"
#include "XYDriver.h"

class VJConfig
{
public:
   VJConfig(const std::filesystem::path &path);

   XYDriverConfig getXYDriverConfig();

   void setXYDriverConfig(const XYDriverConfig &newValue);

private:
   bool getXY(const std::string &name, XY &result);
   void setXY(const std::string &name, const XY &value);

private:
   SQLDB db;
};



#endif
