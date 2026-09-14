import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# ============================================================
# 1. Данные
# ============================================================

p1 = np.array([371.389, 541.821, -2.0])
p2 = np.array([367.657, 559.082, -1.5])
p3 = np.array([399.379, 555.816, -1.5])

g_plane = np.array([0.0030502569715415927, 0.029626531430264426])

# --- градиенты в вершинах (экспериментируйте) ---
g1 = g_plane.copy()
g2 = g_plane.copy()
g3 = g_plane.copy()

bary_target = np.array([0.11289366541628222, 0.64388218769458849, 0.24322414688912930])

# Плоскость (для эталона)
c = p1[2] - g_plane[0]*p1[0] - g_plane[1]*p1[1]
def plane_z(x, y):
    return c + g_plane[0]*x + g_plane[1]*y

def bary_to_xy(u, v, w):
    return u*p1[0] + v*p2[0] + w*p3[0], u*p1[1] + v*p2[1] + w*p3[1]

def bary_to_xyz_plane(u, v, w):
    x, y = bary_to_xy(u, v, w)
    return np.array([x, y, plane_z(x, y)])

# ============================================================
# 2. Макро-контрольные точки степени 3 из градиентов
# ============================================================

def D(grad, pi, pj):
    """Приращение z вдоль ребра pi->pj по градиенту grad."""
    return grad[0]*(pj[0]-pi[0]) + grad[1]*(pj[1]-pi[1])

def edge_pt(Pi, Pj, t, z_val):
    """Точка на ребре Pi-Pj с долей t, z задаётся."""
    x = (1-t)*Pi[0] + t*Pj[0]
    y = (1-t)*Pi[1] + t*Pj[1]
    return np.array([x, y, z_val])

# Вершины
b300 = p1.copy()
b030 = p2.copy()
b003 = p3.copy()

# Ребро p1-p2
b210 = edge_pt(p1, p2, 1/3, p1[2] + (1/3)*D(g1, p1, p2))
b120 = edge_pt(p1, p2, 2/3, p2[2] - (1/3)*D(g2, p1, p2))

# Ребро p2-p3
b021 = edge_pt(p2, p3, 1/3, p2[2] + (1/3)*D(g2, p2, p3))
b012 = edge_pt(p2, p3, 2/3, p3[2] - (1/3)*D(g3, p2, p3))

# Ребро p3-p1
b102 = edge_pt(p3, p1, 1/3, p3[2] + (1/3)*D(g3, p3, p1))
b201 = edge_pt(p3, p1, 2/3, p1[2] - (1/3)*D(g1, p3, p1))

# b111: выбор, воспроизводящий плоскость
x_b111, y_b111 = bary_to_xy(1/3, 1/3, 1/3)
z_b111 = (1/6)*(b210[2]+b120[2]+b021[2]+b012[2]+b102[2]+b201[2])
b111 = np.array([x_b111, y_b111, z_b111])

# Macro dict
B = {
    (3,0,0): b300, (0,3,0): b030, (0,0,3): b003,
    (2,1,0): b210, (1,2,0): b120,
    (0,2,1): b021, (0,1,2): b012,
    (1,0,2): b102, (2,0,1): b201,
    (1,1,1): b111,
}

# Проверка для плоскости
print("Macro control points (z):")
for k, v in B.items():
    print(f"  {k}: z = {v[2]:.8f}")

# ============================================================
# 3. de Casteljau split at centroid
# ============================================================

def casteljau_level1(B):
    L1 = {}
    L1[(2,0,0)] = (B[(3,0,0)] + B[(2,1,0)] + B[(2,0,1)]) / 3
    L1[(0,2,0)] = (B[(0,3,0)] + B[(1,2,0)] + B[(0,2,1)]) / 3
    L1[(0,0,2)] = (B[(0,0,3)] + B[(1,0,2)] + B[(0,1,2)]) / 3
    L1[(1,1,0)] = (B[(2,1,0)] + B[(1,2,0)] + B[(1,1,1)]) / 3
    L1[(1,0,1)] = (B[(2,0,1)] + B[(1,0,2)] + B[(1,1,1)]) / 3
    L1[(0,1,1)] = (B[(0,2,1)] + B[(0,1,2)] + B[(1,1,1)]) / 3
    return L1

def casteljau_level2(L1):
    L2 = {}
    L2[(1,0,0)] = (L1[(2,0,0)] + L1[(1,1,0)] + L1[(1,0,1)]) / 3
    L2[(0,1,0)] = (L1[(1,1,0)] + L1[(0,2,0)] + L1[(0,1,1)]) / 3
    L2[(0,0,1)] = (L1[(1,0,1)] + L1[(0,1,1)] + L1[(0,0,2)]) / 3
    return L2

def casteljau_level3(L2):
    return (L2[(1,0,0)] + L2[(0,1,0)] + L2[(0,0,1)]) / 3

L1 = casteljau_level1(B)
L2 = casteljau_level2(L1)
G_pt = casteljau_level3(L2)

# ============================================================
# 4. Субпатчи: 10 контрольных точек каждый
# ============================================================
# S1 = (p1, p2, G); S2 = (p2, p3, G); S3 = (p3, p1, G)

S1 = {
    (3,0,0): B[(3,0,0)],
    (0,3,0): B[(0,3,0)],
    (0,0,3): G_pt,
    (2,1,0): B[(2,1,0)],
    (1,2,0): B[(1,2,0)],
    (2,0,1): L1[(2,0,0)],
    (1,0,2): L2[(1,0,0)],
    (0,2,1): L1[(0,2,0)],
    (0,1,2): L2[(0,1,0)],
    (1,1,1): L1[(1,1,0)],
}

S2 = {
    (3,0,0): B[(0,3,0)],
    (0,3,0): B[(0,0,3)],
    (0,0,3): G_pt,
    (2,1,0): B[(0,2,1)],
    (1,2,0): B[(0,1,2)],
    (2,0,1): L1[(0,2,0)],
    (1,0,2): L2[(0,1,0)],
    (0,2,1): L1[(0,0,2)],
    (0,1,2): L2[(0,0,1)],
    (1,1,1): L1[(0,1,1)],
}

S3 = {
    (3,0,0): B[(0,0,3)],
    (0,3,0): B[(3,0,0)],
    (0,0,3): G_pt,
    (2,1,0): B[(1,0,2)],
    (1,2,0): B[(2,0,1)],
    (2,0,1): L1[(0,0,2)],
    (1,0,2): L2[(0,0,1)],
    (0,2,1): L1[(2,0,0)],
    (0,1,2): L2[(1,0,0)],
    (1,1,1): L1[(1,0,1)],
}

# ============================================================
# 5. Формула Безье степени 3 (только z)
# ============================================================

ORDER = [(3,0,0),(0,3,0),(0,0,3),
         (2,1,0),(1,2,0),(2,0,1),(1,0,2),
         (0,2,1),(0,1,2),(1,1,1)]

def bernstein(u, v, w):
    return {
        (3,0,0): u**3, (0,3,0): v**3, (0,0,3): w**3,
        (2,1,0): 3*u*u*v, (1,2,0): 3*u*v*v,
        (2,0,1): 3*u*u*w, (1,0,2): 3*u*w*w,
        (0,2,1): 3*v*v*w, (0,1,2): 3*v*w*w,
        (1,1,1): 6*u*v*w,
    }

def eval_bezier_z(ctrl_dict, u, v, w):
    Bp = bernstein(u, v, w)
    return sum(Bp[k] * ctrl_dict[k][2] for k in ORDER)

# ============================================================
# 6. Сетка в субпатче
# ============================================================
# x,y — линейно через макро-барицентрику;
# z   — Безье по ctrl_dict.

def local_to_macro_bary(V1b, V2b, V3b, local_uvw):
    u, v, w = local_uvw
    return (u*V1b[0] + v*V2b[0] + w*V3b[0],
            u*V1b[1] + v*V2b[1] + w*V3b[1],
            u*V1b[2] + v*V2b[2] + w*V3b[2])

def make_bary_grid(n=40):
    out = []
    for i in range(n+1):
        for j in range(n+1-i):
            k = n - i - j
            out.append((i/n, j/n, k/n))
    return out

def surface_grid_sub(V1b, V2b, V3b, ctrl_dict, n=40):
    out = []
    for (u, v, w) in make_bary_grid(n):
        um, vm, wm = local_to_macro_bary(V1b, V2b, V3b, (u, v, w))
        x, y = bary_to_xy(um, vm, wm)
        z = eval_bezier_z(ctrl_dict, u, v, w)
        out.append((x, y, z))
    return np.array(out)

# Барицентрика вершин субпатчей в макро
B_P1 = (1, 0, 0)
B_P2 = (0, 1, 0)
B_P3 = (0, 0, 1)
B_G  = (1/3, 1/3, 1/3)

S1_surf = surface_grid_sub(B_P1, B_P2, B_G, S1)
S2_surf = surface_grid_sub(B_P2, B_P3, B_G, S2)
S3_surf = surface_grid_sub(B_P3, B_P1, B_G, S3)

# ============================================================
# 7. Визуализация
# ============================================================

fig = plt.figure(figsize=(16, 6))
ax = fig.add_subplot(121, projection='3d')

ax.scatter(S1_surf[:,0], S1_surf[:,1], S1_surf[:,2], c='red',   s=3, label='S1')
ax.scatter(S2_surf[:,0], S2_surf[:,1], S2_surf[:,2], c='green', s=3, label='S2')
ax.scatter(S3_surf[:,0], S3_surf[:,1], S3_surf[:,2], c='blue',  s=3, label='S3')

for P in [p1, p2, p3]:
    ax.scatter([P[0]],[P[1]],[P[2]], c='k', s=60, marker='o')
ax.scatter([G_pt[0]],[G_pt[1]],[G_pt[2]], c='orange', s=80, marker='D', label='G')

P_target = bary_to_xyz_plane(*bary_target)
ax.scatter([P_target[0]],[P_target[1]],[P_target[2]],
           c='magenta', s=120, marker='*', label='target')

ax.set_xlabel('X'); ax.set_ylabel('Y'); ax.set_zlabel('Z')
ax.set_title(f'de Casteljau split,  g1==g2==g3==g_plane: {np.allclose(g1,g_plane) and np.allclose(g2,g_plane) and np.allclose(g3,g_plane)}')
ax.legend(loc='upper left', fontsize=8)

xs = [p1[0], p2[0], p3[0]]
ys = [p1[1], p2[1], p3[1]]
zs = [plane_z(x, y) for x, y in zip(xs, ys)]
pad = 0.5
ax.set_xlim(min(xs)-pad, max(xs)+pad)
ax.set_ylim(min(ys)-pad, max(ys)+pad)
ax.set_zlim(min(zs)-pad, max(zs)+pad)

ax2 = fig.add_subplot(122)
for surf, col, lab in [(S1_surf,'red','S1'),
                       (S2_surf,'green','S2'),
                       (S3_surf,'blue','S3')]:
    dz = surf[:,2] - plane_z(surf[:,0], surf[:,1])
    cx, cy = surf[:,0].mean(), surf[:,1].mean()
    rad = np.sqrt((surf[:,0]-cx)**2 + (surf[:,1]-cy)**2)
    ax2.scatter(rad, dz*1e3, c=col, s=4, label=lab)

ax2.axhline(0, color='k', lw=0.8)
ax2.set_xlabel('radial distance from subpatch center (xy)')
ax2.set_ylabel('dz = z_bezier - z_plane  (×1e-3)')
ax2.set_title('Deviation from reference plane')
ax2.legend(); ax2.grid(True, alpha=0.3)

plt.tight_layout()
plt.show()

# ============================================================
# 8. Проверка
# ============================================================

print("\nMax |dz| from reference plane:")
for surf, lab in [(S1_surf,'S1'),(S2_surf,'S2'),(S3_surf,'S3')]:
    dz = surf[:,2] - plane_z(surf[:,0], surf[:,1])
    print(f"  {lab}: max|dz| = {np.max(np.abs(dz)):.3e}")

# Проверка C1 на внутренних рёбрах
def bezier_partial_u(ctrl_dict, u, v, w):
    """dP/du для кубического треугольника Безье (только z)."""
    # Для степени 3: dP/du = 3 * sum b^(1)_{i,j,k} * B_deg2_{ijk}
    # b^(1)_{ijk} = b_{i+1,j,k} (сдвиг индекса по u)
    # Используем формулы для dP/du через контрольные точки степени 2
    # Проще: численная производная
    h = 1e-6
    z1 = eval_bezier_z(ctrl_dict, u+h, v-h/2, w-h/2)
    z2 = eval_bezier_z(ctrl_dict, u-h, v+h/2, w+h/2)
    return (z1 - z2) / (2*h)  # приблизительно

# Внутренние рёбра субпатчей идут от вершины к G.
# Их направление в плоскости xy — (G_xy - p_i_xy).
# Производная вдоль ребра должна совпадать у соседних субпатчей.

print("\nC1 check on internal edges (dz/ds должен совпадать):")
# Ребро p1-G: между S1 и S3
for edge_name, Pi_bary, surfA, surfB, subA, subB in [
    ('p1-G', B_P1, S1, S3, (B_P1,B_P2,B_G), (B_P3,B_P1,B_G)),
    ('p2-G', B_P2, S2, S1, (B_P2,B_P3,B_G), (B_P1,B_P2,B_G)),
    ('p3-G', B_P3, S3, S2, (B_P3,B_P1,B_G), (B_P2,B_P3,B_G)),
]:
    # средняя точка на ребре в локальных коорд. каждого субпатча
    # (0.5, 0, 0.5) — середина ребра V1-V3 для первого
    # нужно проверить, что это соответствует (0.5, 0.5, 0) для второго
    # для конкретности возьмём точку в середине ребра в макро
    mid_bary = tuple(0.5*a + 0.5*b for a, b in zip(Pi_bary, B_G))
    xm, ym = bary_to_xy(*mid_bary)
    # Направление вдоль ребра от Pi к G в xy
    Gx, Gy = bary_to_xy(*B_G)
    Pi_x, Pi_y = bary_to_xy(*Pi_bary)
    dirx, diry = Gx - Pi_x, Gy - Pi_y
    L = np.hypot(dirx, diry)
    dirx, diry = dirx/L, diry/L

    # Численно берём точки чуть в стороны от середины вдоль ребра
    eps = 1e-4
    t1, t2 = 0.5 - eps, 0.5 + eps
    b1 = tuple((1-t)*a + t*b for a, b in zip(Pi_bary, B_G))
    b2 = tuple((1-t)*a + t*b for a, b in zip(Pi_bary, B_G))
    x1, y1 = bary_to_xy(*b1)
    x2, y2 = bary_to_xy(*b2)

    # Значения z через каждый из двух субпатчей в этих точках
    # локальные коорд. для surfA: t1 соответствует (1-t1, 0, t1)?
    # Проверим: mid_bary = (1-t)*Pi + t*G.
    # для S1 (B_P1,B_P2,B_G) mid_bary = u'*B_P1 + v'*B_P2 + w'*B_G.
    # Нужно найти (u',v',w') такие что это = (1-t)*B_P1 + t*B_G.
    # => u' = 1-t, v' = 0, w' = t.
    # Для subB нужно найти (u'',v'',w'') в (V1',V2',V3') субпатча B.
    # Это линейная система, но проще: mid_bary в макро = та же точка,
    # решим для каждого субпатча отдельно.

    def macro_to_local(macro_bary, V1b, V2b, V3b):
        # Решаем [V1b V2b V3b]^T * (u,v,w)^T = macro_bary
        M = np.array([V1b, V2b, V3b]).T
        return np.linalg.solve(M, np.array(macro_bary))

    # Возьмём точки чуть по обе стороны от середины
    # параметр t вдоль ребра: 0.5±eps
    # но для вычисления производной в самой середине лучше
    # использовать центральную разность

    # Значение z из surfA и surfB в тех же xy
    # т.к. mid_bary одно и то же, локальные коорд. разные
    localA1 = macro_to_local(b1, *subA)
    localA2 = macro_to_local(b2, *subA)
    localB1 = macro_to_local(b1, *subB)
    localB2 = macro_to_local(b2, *subB)

    zA1 = eval_bezier_z(surfA, *localA1)
    zA2 = eval_bezier_z(surfA, *localA2)
    zB1 = eval_bezier_z(surfB, *localB1)
    zB2 = eval_bezier_z(surfB, *localB2)

    # Производные вдоль ребра (на единицу параметра t)
    dA = (zA2 - zA1) / (2*eps)
    dB = (zB2 - zB1) / (2*eps)

    # Значения z в середине
    localA0 = macro_to_local(mid_bary, *subA)
    localB0 = macro_to_local(mid_bary, *subB)
    zA0 = eval_bezier_z(surfA, *localA0)
    zB0 = eval_bezier_z(surfB, *localB0)

    print(f"  {edge_name}: z_A={zA0:.6f}, z_B={zB0:.6f}, "
          f"dz={zA0-zB0:.2e}, dA/ds={dA:.6f}, dB/ds={dB:.6f}, diff={dA-dB:.2e}")
    