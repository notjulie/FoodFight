
#include "VJConfig.h"


VJConfig::VJConfig(const std::filesystem::path &path)
{
   // create the directory if it doesn't exist
   std::filesystem::create_directories(path.parent_path());

   // create or open the DB
   db.OpenOrCreate(path.string());

   // create the schema
   db.ExecuteNonQuery("CREATE TABLE IF NOT EXISTS XYConfig (Corner TEXT, X REAL, Y REAL)");
}


XYDriverConfig VJConfig::getXYDriverConfig()
{
   // start with the default
   XYDriverConfig result;

   // override with any values we have
   getXY("00", result.xy00);
   getXY("01", result.xy01);
   getXY("10", result.xy10);
   getXY("11", result.xy11);
   return result;
}


void VJConfig::setXYDriverConfig(const XYDriverConfig &newValue)
{
   setXY("00", newValue.xy00);
   setXY("01", newValue.xy01);
   setXY("10", newValue.xy10);
   setXY("11", newValue.xy11);
}


bool VJConfig::getXY(const std::string &name, XY &result)
{
   SQLStatement queryResult = db.ExecuteQuery("SELECT X,Y FROM XYConfig WHERE Corner = ?", name);
   if (!queryResult.MoveNext())
      return false;
   result.x = queryResult.GetColumn(0);
   result.y = queryResult.GetColumn(1);
   return true;
}


void VJConfig::setXY(const std::string &name, const XY &value)
{
   db.ExecuteTransaction([=](){
      db.ExecuteNonQuery("DELETE FROM XYConfig WHERE Corner = ?", name);
      db.ExecuteNonQuery("INSERT INTO XYConfig (Corner,X,Y) VALUES (?,?,?)", name, value.x, value.y);
   });
}

