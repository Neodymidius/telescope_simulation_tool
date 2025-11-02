# Physics based ray tracing for telescopes

## Abstract

Simulations are essential for designing, validating, and interpreting space-based telescopes, whose optics are unreachable after launch. The widely used SIXTE framework currently images photons via precomputed point-spread functions (PSFs), limiting it to calibrated instruments and obscuring effects such as stray light, ghost rays, and single reflections. This project replaces that stage with a physics-based Monte Carlo ray-tracing module that models full optical assemblies and predicts performance without external calibration data.

The module is architected as an extensible library integrated into SIXTE via a strategy pattern and human-readable XML configuration. It supports analytic Wolter-I optics (paraboloid/hyperboloid surfaces) and a novel, efficient Lobster-Eye approach that traces rays through a representative pore in a local frame with constrained Euler rotations, avoiding explicit simulation of billions of channels. Reflectivity and grazing-incidence scattering are modeled with microfacet surface theory (Beckmann/GGX), and non-reflective structures (e.g., spiders, baffles) are imported as meshes. Intel Embree accelerates intersection finding, yielding large performance gains over a prior prototype.


## Future development

### Code

* Refactor for parallel compuatation needed

* Adding new telescope architecture: Not stopping at X-ray but now going also optical with Newton telescope configuration.



