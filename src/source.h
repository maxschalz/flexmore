#ifndef CYCLUS_FLEXMORE_SOURCE_H_
#define CYCLUS_FLEXMORE_SOURCE_H_

#include <set>
#include <vector>
#include <string>

#include "cyclus.h"

namespace flexmore {

/// @class Source
///
/// This Facility is intended
/// as a skeleton to guide the implementation of new Facility
/// agents.
/// The Source class inherits from the Facility class and is
/// dynamically loaded by the Agent class when requested.
///
/// @section intro Introduction
/// Place an introduction to the agent here.
///
/// @section agentparams Agent Parameters
/// Place a description of the required input parameters which define the
/// agent implementation.
///
/// @section optionalparams Optional Parameters
/// Place a description of the optional input parameters to define the
/// agent implementation.
///
/// @section detailed Detailed Behavior
/// Place a description of the detailed behavior of the agent. Consider
/// describing the behavior at the tick and tock as well as the behavior
/// upon sending and receiving materials and messages.
class Source : public cyclus::Facility,
  public cyclus::toolkit::CommodityProducer,
  public cyclus::toolkit::Position {
  friend class SourceTest;
 public:
  /// Constructor for Source Class
  /// @param ctx the cyclus context for access to simulation-wide parameters
  explicit Source(cyclus::Context* ctx);
  
  virtual ~Source();

  #pragma cyclus note { \
    "doc":  "A stub facility is provided as a skeleton " \
            "for the design of new facility agents.", \
  }
  
  #pragma cyclus def clone
  #pragma cyclus def schema
  #pragma cyclus def annotations
  #pragma cyclus def infiletodb
  #pragma cyclus def snapshot
  #pragma cyclus def snapshotinv
  #pragma cyclus def initinv

  virtual void InitFrom(Source* m);
  virtual void InitFrom(cyclus::QueryableBackend* b);
  virtual void EnterNotify();
  virtual std::string str();
  virtual void Tick();
  virtual void Tock() {};
  
  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
      GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
                  commod_request);
  
  virtual void GetMatlTrades(
    const std::vector<cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
    cyclus::Material::Ptr> >& responses
  );
  
 private:
  
  double currentThroughput;
  
  void RecordPosition();
  void SetThroughput();
  
  #pragma cyclus var { \
    "tooltip": "source output commodity", \
    "doc": "Output commodity on which the source offers material.", \
    "uilabel": "Output Commodity", \
    "uitype": "outcommodity", \
  }
  std::string outcommod;
  
  #pragma cyclus var { \
    "tooltip": "name of material recipe to provide", \
    "doc": "Name of composition recipe that this source provides regardless " \
           "of requested composition. If empty, source creates and provides " \
           "whatever compositions are requested.", \
    "default": "", \
    "uilabel": "Output Recipe", \
    "uitype": "outrecipe", \
  }
  std::string outrecipe;

  #pragma cyclus var { \
    "doc": "Total amount of material this source has remaining. " \
           "Every trade decreases this value by the supplied material " \
           "quantity. When it reaches zero, the source cannot provide " \
           "any more material.", \
    "default": 1e299, \
    "uilabel": "Initial Inventory", \
    "uitype": "range", \
    "range" : [0.0, 1e299], \
    "units" : "kg", \
  }
  double inventory_size;
  
  #pragma cyclus var { \
    "tooltip": "throughput list", \
    "doc": "amount of commodity that can be supplied at each time step"\
           "and where each entry of the list is only valid for one time"\
           "step. If there is only one element in the list, then this value"\
           "is used for all timesteps, else, the length of the list has to "\
           "be identical to the duration of the simulation. Moreover, "\
           "all values have to be positive or zero.", \
    "default": [1e299], \
    "uilabel": "Maximum throughput list", \
    "uitype": "oneormore", \
    "units": "kg/time step", \
  }
  std::vector<double> throughput;

  #pragma cyclus var { \
    "tooltip": "geographical latitude", \
    "doc": "Latitude of the agent's geographical position. The " \
           "value should be expressed in degrees as a double.", \
    "default": 0.0, \
    "uilabel": "Geographical latitude in degrees as a double", \
  }
  double latitude;

  #pragma cyclus var { \
    "tooltip": "geographical longtitude", \
    "doc": "Longitude of the agent's geographical position. The " \
           "value should be expressed in degrees as a double.", \
    "default": 0.0, \
    "uilabel": "Geographical longitude in degrees as a double", \
  }
  double longitude;
  
  cyclus::toolkit::Position coordinates;
};

}  // namespace flexmore

#endif  // CYCLUS_FLEXMORE_SOURCE_H_
