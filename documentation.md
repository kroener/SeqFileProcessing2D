# Documentation
This document will give a short description of the workflow to extract tracks from recordings.

## Usual Workflow
The workflow can be split in five steps:
 1. Segmentation: In this step difference images are analyzed and mosquito contours and positions are estimated
 2. Cleaning: In this step wrong detected Mosquitoes can be removed.
 3. Tracking: Tracking connects the found mosquito positions to form tracks.
 4. Cleaning: Tracks can be connected and points can be added to tracks.
 5. Exporting: The resulting position and track files can be exported. A simple visualisation is also included to save an overview image over the tracks in the observed time frame.
 
## Parameter
Step 2. and 3. require some parameters to be adjusted. 
### Segmentation
**MinArea**: Minimum number of pixels for a blob
**MaxArea**: Maximum number of pixels for a blob
**Threshold**: A histogram of the difference is created and a grey scale difference threshold is calculated by taking the x percent of the distance between the value with the maximum counts to the maximum value with least a count of one.    
**MinThreshold**: If the Threshold is below MinThreshold, Threshold will be set to minThreshold  
**Use the ith**: Generate difference from image(n) and image(n-i)  
**Remove BG**: Shrink the blobs by eroding with an n sized square, than dilating with the same sized stancil  
**Add FG**: Same as Remove BG, but first dilate, than erode.  
**ClusterDist**: This discards points too close together. Keeping only the stronger one in a radius of x   
**Display each N**: For displaying, to speed up processing, you can skip displaying frames. Only each n-th is shown. 0 disables display completely.  
**Black On White**: Select whether to search dark spots on light background or light spots on dark backgrounds.  
**medianBlur1**: Apply median blur to input images before building difference image.  
**medianBlur2**: Apply median blur to the difference image.  
**gaussian kernel 1**: Apply gaussian kernel to images before creating the difference image  
**gaussian sigma 1**: "   
**gaussian kernel 2**: Applying gaussian kernel to the difference image  
**gaussian sigma 2**: "  
**Mask Threshold**: A threshold is applied to the original pictures, only pixels from 0 to threshold or (if a negative value is specified) from -threshold to 255 are selected.  


### Tracking
**MinDist**: Minimum distance travelled between frames   
**MaxDist**: Maximum distance travelled between frames  
**MaxGap**: Number of frames, which can be skipped while looking for the next Mosquito position  
**MinArea**: Filter for the minimum area of the Mosquito's blob size  
**MaxArea**: Filter for the maximum area of the Mosquito's blob size  
**MaxNeigh**: Number of positions closest to the initial search position to analyse  
**MaxCohLen**: Taking the maximum coherence length number of historic frames for building the prediction of the mosquitoes next position  
**MinTrDist**:Minimum number of historical points to estimate the next position using temporal coherence  
**T.D.L.**: Track display length, display this number long tail to the current frame in the viewer  
**m.T.D.L.**: Minimum track display length, show only tracks with at least this number of points  
**append**: Overwrite or append the tracking results  
**MaxDist(aj)**: For the autojoiner, set maximum distance between track end and track start for tracks to join  
**MaxGap(aj)**: For the autojoiner, set the maximum number of skipped frames  
