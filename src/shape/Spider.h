//
// Created by neo on 07.11.24.
//

#ifndef SIXTE_SPIDER_H
#define SIXTE_SPIDER_H

#include "geometry/Vec3fa.h"
#include <string>

class Spider {
public:
    Spider();
    explicit Spider(std::string  filename, const Vec3fa& position);
    std::string filename;
    Vec3fa position;
    unsigned int geomID = -1;


};


#endif //SIXTE_SPIDER_H
