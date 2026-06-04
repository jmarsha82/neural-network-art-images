# Architecture Notes

## Initial Neural Network Strategy

The first version uses **OpenCV DNN** with an ONNX image model. This is the simplest open-source path for a CPU-only C++ desktop app:

- No paid libraries.
- No CUDA requirement.
- ONNX gives access to many pretrained open-source models.
- OpenCV also handles image loading and preprocessing.

The app uses the model output for both:

- **Classification**: top labels and confidence scores.
- **Similarity search**: vector comparison with cosine similarity.

For stronger visual similarity later, use an ONNX model that exposes an embedding layer, or add a dedicated embedding model such as CLIP/ViT exported to ONNX.

## Why Not LibTorch First?

LibTorch is excellent when the project needs PyTorch-native model execution, custom tensors, or fine-tuning/training. For this first version, the user chose a pretrained CPU-only model, so OpenCV DNN has a smaller install surface and is easier to ship.

LibTorch still makes sense as a later optional runtime if we add:

- Fine-tuning on folder labels.
- PyTorch `.pt` / TorchScript model loading.
- More advanced training dashboards.

## Clustering

Clustering is handled by `Clusterer`.

- With `ARTNET_USE_MLPACK=ON`, it uses **mlpack KMeans**.
- Without mlpack, it uses a small built-in k-means fallback so the app can still run.

## UI

The UI is Qt Widgets rather than a web UI because the goal is a native C++ desktop app. The first screen is the actual image/network workspace:

- image list
- preview
- prediction table
- similar image table
- scan, inference, similarity, and export actions

## Data Root

The default library root is:

```text
C:\Users\jmars\OneDrive\Pictures\Art Images
```

Folders are treated as optional labels. This supports the mixed-folder choice: useful folders can act as labels, while unhelpful folders still contribute images for browsing/search/clustering.
