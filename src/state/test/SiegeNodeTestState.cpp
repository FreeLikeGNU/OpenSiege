
#include "SiegeNodeTestState.hpp"

#include <sstream>

#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/Group>
#include <osg/PolygonMode>
#include <osg/ComputeBoundsVisitor>
#include <osgViewer/Viewer>
#include <spdlog/spdlog.h>

#include "osg/ReaderWriterSNO.hpp"

namespace ehb
{
    // https://github.com/xarray/osgRecipes/blob/master/cookbook/chapter8/ch08_07/OctreeBuilder.cpp
    // flipped the parameters for ease of use with ds bboxes
    static osg::Group* createBoxForDebug(const osg::Vec3& min, const osg::Vec3& max)
    {
        osg::Vec3 dir = max - min;
        osg::ref_ptr<osg::Vec3Array> va = new osg::Vec3Array(10);
        (*va)[0] = min + osg::Vec3(0.0f, 0.0f, 0.0f);
        (*va)[1] = min + osg::Vec3(0.0f, 0.0f, dir[2]);
        (*va)[2] = min + osg::Vec3(dir[0], 0.0f, 0.0f);
        (*va)[3] = min + osg::Vec3(dir[0], 0.0f, dir[2]);
        (*va)[4] = min + osg::Vec3(dir[0], dir[1], 0.0f);
        (*va)[5] = min + osg::Vec3(dir[0], dir[1], dir[2]);
        (*va)[6] = min + osg::Vec3(0.0f, dir[1], 0.0f);
        (*va)[7] = min + osg::Vec3(0.0f, dir[1], dir[2]);
        (*va)[8] = min + osg::Vec3(0.0f, 0.0f, 0.0f);
        (*va)[9] = min + osg::Vec3(0.0f, 0.0f, dir[2]);

        osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
        geom->setVertexArray(va.get());
        geom->addPrimitiveSet(new osg::DrawArrays(GL_QUAD_STRIP, 0, 10));

        osg::ref_ptr<osg::Group> group = new osg::Group;
        group->addChild(geom.get());
        group->getOrCreateStateSet()->setAttribute(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE));
        group->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        return group.release();
    }

    static osg::Group* createBoxForDebug(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
    {
        return createBoxForDebug(osg::Vec3(minX, minY, minZ), osg::Vec3(maxX, maxY, maxY));
    }

    void SiegeNodeTestState::enter()
    {
        auto log = spdlog::get("game");

        static const std::string meshName = "t_grs01_houses_generic-a-log.sno";

        mesh = dynamic_cast<SiegeNodeMesh*>(osgDB::readNodeFile(meshName));
        if (mesh != nullptr)
        {
            log->info("Loaded {}", meshName);

            auto t1 = new osg::MatrixTransform;
            t1->addChild(mesh);

            // TODO: is there a better way to do this?
            // re-position the camera based on the size of node and orient it up a little bit get a birds eye-view
            if (auto manipulator = viewer.getCameraManipulator())
            {
                osg::ComputeBoundsVisitor cbv;
                mesh->accept(cbv);

                t1->addChild(createBoxForDebug(cbv.getBoundingBox()._min, cbv.getBoundingBox()._max));

                osg::BoundingSphere sphere;
                sphere.expandBy(cbv.getBoundingBox());

                double radius = osg::maximum(double(sphere.radius()), 1e-6);
                double dist = 3.5f * radius;

                manipulator->setHomePosition(sphere.center() + osg::Vec3d(0.0, dist, 7.0f), sphere.center(), osg::Vec3d(0.0f, 0.0f, 1.0f));
                manipulator->home(1);
            }

            auto t2 = new osg::MatrixTransform;
            t2->addChild(mesh);

            SiegeNodeMesh::connect(t1, 4, t2, 8);

            scene.addChild(t1);
            scene.addChild(t2);
        }
        else
        {
            log->error("Failed to load {}", meshName);
        }
    }

    void SiegeNodeTestState::leave()
    {
        scene.removeChildren(0, scene.getNumChildren());
    }

    void SiegeNodeTestState::update(double deltaTime)
    {
    }

    bool SiegeNodeTestState::handle(const osgGA::GUIEventAdapter & event, osgGA::GUIActionAdapter & action)
    {
        switch (event.getEventType())
        {
            case (osgGA::GUIEventAdapter::KEYUP):
            {
                if (event.getKey() == '1')
                {
                    mesh->toggleAllDoorLabels();
                }
            }
        }

        return false;
    }
}
