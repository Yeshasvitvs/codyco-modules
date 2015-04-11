dofile('./gas_funcs.lua')

require('math')

tol = 1e-15;

function createRandomPointCoord()
    local point = PointCoord.create()
    point.x = math.random()
    point.y = math.random()
    point.z = math.random()
    return point;
end

function rotX(theta)
    local costheta = math.cos(theta)
   local sintheta = math.sin(theta)
   local rot = RotMatrix.create()
   rot.yy = costheta;
   rot.yz = -sintheta;
   rot.zy = sintheta;
   rot.zz = costheta; 
   return rot
end

function rotY(theta)
    local costheta = math.cos(theta)
    local sintheta = math.sin(theta)
    local rot = RotMatrix.create()
    rot.xx = costheta
    rot.xz = sintheta
    rot.zx = -sintheta 
    rot.zz = costheta
    return rot
end

function rotZ(theta)
    local costheta = math.cos(theta)
    local sintheta = math.sin(theta)
    local rot = RotMatrix.create()
    rot.xx = costheta;
    rot.xy = -sintheta;
    rot.yx = sintheta;
    rot.yy = costheta;
    return rot;
end

function createRandomRotMatrix()
    local randRotX = rotX(math.random())
    local randRotY = rotY(math.random())    
    local randRotZ = rotZ(math.random())
    rot = randRotX:compose(randRotY:compose(randRotZ))
    return rot
end

function createRandomHomTransform()
    local trans = HomTransform.create()
    trans.rot = createRandomRotMatrix()
    trans.origin = createRandomPointCoord()
    return trans
end 

function checkZeroPoint(point)
    ok = true
    if( math.abs(point.x) > tol ) then
        ok = false
    end
    if( math.abs(point.y) > tol ) then
        ok = false
    end
    if( math.abs(point.z) > tol ) then
        ok = false
    end

    if( not ok ) then
        point:print("[ERROR] checkZeroPoint failed with PointCoord:")
    end

    return ok
end


function checkRotIdentity(rot)
    ok = true
    if( math.abs(rot.xx-1) > tol ) then
        ok = false
    end
    if( math.abs(rot.yy-1) > tol ) then
        ok = false
    end
    if( math.abs(rot.zz-1) > tol ) then
        ok = false
    end
    if( math.abs(rot.xy) > tol ) then
        ok = false
    end
    if( math.abs(rot.xz) > tol ) then
        ok = false
    end
    if( math.abs(rot.yx) > tol ) then
        ok = false
    end
    if( math.abs(rot.yz) > tol ) then
        ok = false
    end
    if( math.abs(rot.zx) > tol ) then
        ok = false
    end
    if( math.abs(rot.zy) > tol ) then
        ok = false
    end

    if( not ok ) then
        rot:print("[ERROR] checkRotIdentity failed with RotMatrix:")
    end

    return ok
end

function checkIdentity(homtransform)
    return checkRotIdentity(homtransform.rot) and checkZeroPoint(homtransform.origin)
end

function testHomTransformInverse()
    transform = createRandomHomTransform();
    invTransform = transform:inverse()
    id1 = transform:compose(invTransform)
    id2 = invTransform:compose(transform)
    assert(checkIdentity(id1))
    assert(checkIdentity(id2))
    print("testHomTransformInverse was ok")
end

function testPointTransformAndBack()
    point1 = createRandomPointCoord();
    transform = createRandomHomTransform();
    invTransform = transform:inverse();
    point2 = invTransform:apply(transform:apply(point1))
    point3 = PointCoord.create()
    point3.x = point1.x-point2.x
    point3.y = point1.y-point2.y
    point3.z = point1.z-point2.z
    assert(checkZeroPoint(point3))
    print("testPointTransformAndBack was ok")
end

function main()
    testHomTransformInverse()
    testPointTransformAndBack()
end


main()























