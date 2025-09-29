//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#include <csignal>
#include <iostream>
#include <stdexcept>

#include "LibCamera/LibCameraFrameGrabber.h"

// project includes
#include "CommandProcessor.h"
#include "FrameHandler.h"
#include "LedControl.h"
#include "SocketListener.h"
#include "SPIDAC.h"
#include "VJConfig.h"
#include "XYDriver.h"


static const std::filesystem::path CONFIG_FILE_PATH = "/usr/local/share/VideoJoystick/config.sql";


extern "C" {
   static void signal_handler(int signal_number);
};


static volatile std::sig_atomic_t signalStatus = 0;

/**
 * Handler for sigint signals
 *
 * @param signal_number ID of incoming signal.
 *
 */
static void signal_handler(int signal_number)
{
   signalStatus = signal_number;
   std::signal(signal_number, SIG_IGN);
}


/**
 * main
 */
int main(int argc, const char **argv)
{
   // load our config
   VJConfig config(CONFIG_FILE_PATH);

   // create a command handler; we can add commands to it as we go along
   CommandProcessor commander;

   // set up our socket listener and tell it how to process commands it receives...
   // this is basically a TCP command line for diagnostics
   SocketListener socketListener([&commander](const std::string &s)
   {
      return commander.ProcessCommand(s);
   });

   // add our command handlers
   commander.AddHandler("shutdown", [](std::string)
   {
      signalStatus = SIGINT;
      return std::string();
   });

   // initialize the SPIDAC and add a couple commands
   SPIDAC spiDac;
   commander.AddHandler("setX", [&spiDac](std::string param)
   {
      spiDac.sendX(atof(param.c_str()));
      return std::string();
   });
   commander.AddHandler("setY", [&spiDac](std::string param)
   {
      spiDac.sendY(atof(param.c_str()));
      return std::string();
   });

   // testing... a loop that lets the SocketListener run but doesn't
   // bother with any of the camera stuff
#if 0
   for (;;)
   {
      if (terminateRequested)
         return 0;
      std::this_thread::sleep_for(std::chrono::seconds(1));
   }
#endif

   // FrameHandler is where the fun begins... it turns incoming frames into
   // pixel locations of the red dot
   FrameHandler frameHandler;
   commander.AddHandler("getImage", [&](std::string)
   {
      return frameHandler.GetImageAsString();
   });
   commander.AddHandler("getPixXY", [&](std::string)
   {
      return std::to_string(frameHandler.getX()) + "," + std::to_string(frameHandler.getY());
   });
   commander.AddHandler("getSaturation", [&frameHandler](std::string){ return std::to_string(frameHandler.getSaturiationPercent()); });
   commander.AddHandler("getFrameProcessTime", [&frameHandler](std::string){ return std::to_string(frameHandler.getFrameProcessTime().count()); });

   // ============================================================
   // Initialize XYDriver
   //
   // XYDriver takes the calculated pixel location and turns it into a
   // joystick position
   // ============================================================
   XYDriver xyDriver;
   xyDriver.setConfig(config.getXYDriverConfig());
   frameHandler.setFrameNotify([&](int pixelX,  int pixelY){
      XY xy = xyDriver.getXY(XY(pixelX, pixelY));
      spiDac.sendX(xy.x);
      spiDac.sendY(xy.y);
   });
   commander.AddHandler("getXY", [&](std::string) {
      XY xy = xyDriver.getXY(XY(frameHandler.getX(), frameHandler.getY()), true);
      return std::to_string(xy.x) + "," + std::to_string(xy.y);
   });
   commander.AddHandler("cal00", [&](std::string) {
      xyDriver.cal00();
      config.setXYDriverConfig(xyDriver.getConfig());
      return std::string();
      });
   commander.AddHandler("cal01", [&](std::string) {
      xyDriver.cal01();
      config.setXYDriverConfig(xyDriver.getConfig());
      return std::string();
      });
   commander.AddHandler("cal10", [&](std::string) {
      xyDriver.cal10();
      config.setXYDriverConfig(xyDriver.getConfig());
      return std::string();
      });
   commander.AddHandler("cal11", [&](std::string) {
      xyDriver.cal11();
      config.setXYDriverConfig(xyDriver.getConfig());
      return std::string();
      });


   std::signal(SIGINT, signal_handler);
   std::signal(SIGTERM, signal_handler);
   std::signal(SIGKILL, signal_handler);

   // Now set up our components
   LedControl::getInstance()->setLedOn(true);

   try
   {
      std::unique_ptr<FrameGrabber> frameGrabber(LibCameraFrameGrabber::createUniqueCamera());

      // Enable the camera video port and tell it its callback function
      frameGrabber->SetupFrameCallback([&](const std::shared_ptr<VideoFrame> &frame)
      {
         // process it
         frameHandler.HandleFrame(frame);
      });

      // start grabbing frames
      frameGrabber->startCapturing();

      // watch for signal to exit
      while (!signalStatus)
      {
         std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
   }
   catch (std::exception &e)
   {
      std::cerr << e.what() << std::endl;
   }

   return 0;
}

