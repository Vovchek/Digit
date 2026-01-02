bool XYPolygon::isInside(const XYPoint& P) const
{
    int NPnt = GetSize();
    double Phi = 0.;

    for (int i = 0; i < NPnt - 1; i++)
    {
        XYPoint v1 = ArrPnt[i] - P;
        XYPoint v2 = ArrPnt[i + 1] - P;

        // Check if P is exactly at a vertex
        if (Distance(v1, XYPoint(0, 0)) < PRECISION)
            return true;  // On vertex

        // Check if P is on edge (vectors are collinear and opposite)
        double cross = v1.X * v2.Y - v1.Y * v2.X;
        if (fabs(cross) < PRECISION) {
            // Collinear - check if between vertices
            double dot = v1.X * v2.X + v1.Y * v2.Y;
            if (dot < 0) return true;  // Opposite directions = on edge
        }

        Phi += Angle(v2, v1);
    }

    // Now check winding number with proper tolerance
    const double TWO_PI = 6.283185307179586;
    if (fabs(fabs(Phi) - TWO_PI) < 0.01)  // Close to 2π
        return true;  // Boundary case
    else if (fabs(Phi) < 0.0001)
        return false; // Outside
    return true;     // Inside
}