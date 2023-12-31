# MaxPoolBackprop {#dev_guide_op_maxpoolbackprop}

**Versioned name**: *MaxPoolBackprop-1*

**Category**: *Pooling*

**Short description**:
[Reference](http://caffe.berkeleyvision.org/tutorial/layers/pooling.html)

**Detailed description**:
[Reference](http://cs231n.github.io/convolutional-networks/#pool)

## Attributes

* *strides*

  * **Description**: *strides* is a distance (in pixels) to slide the window on
    the feature map over the (z, y, x) axes for 3D poolings and (y, x) axes for
    2D poolings. For example, *strides* equal "4,2,1" means sliding the window 4
    pixel at a time over depth dimension, 2 over height dimension and 1 over
    width dimension.
  * **Range of values**: Non-negative s64 values.
  * **Type**: s64[]
  * **Required**: *yes*

* *pads_begin*

  * **Description**: *pads_begin* is a number of pixels to add to the beginning
    along each axis. For example, *pads_begin* equal "1,2" means adding 1 pixel
    to the top of the input and 2 to the left of the input.
  * **Range of values**: Non-negative s64 values.
  * **Type**: s64[]
  * **Required**: *yes*
  * **Note**: the attribute is ignored when *auto_pad* attribute is specified.

* *pads_end*

  * **Description**: *pads_end* is a number of pixels to add to the ending along
    each axis. For example, *pads_end* equal "1,2" means adding 1 pixel to the
    bottom of the input and 2 to the right of the input.
  * **Range of values**: Non-negative s64 values.
  * **Type**: s64[]
  * **Required**: *yes*
  * **Note**: the attribute is ignored when *auto_pad* attribute is specified.

* *kernel*

  * **Description**: *kernel* is a size of each filter. For example, *kernel*
    equal (2, 3) means that each filter has height equal to 2 and width equal
    to 3.
  * **Range of values**: positive s64 values.
  * **Type**: s64[]
  * **Required**: *yes*

* *auto_pad*

  * **Description**: *auto_pad* how the padding is calculated. Possible values:

    * *none (not specified)*: use explicit padding values.
    * *same_upper (same_lower)* the input is padded to match the output size.
      In case of odd padding value an extra padding is added at the end (at the
      beginning).
    * *valid* - do not use padding.

  * **Type**: string
  * **Default value**: *none*
  * **Required**: *no*
  * **Note**: *pads_begin* and *pads_end* attributes are ignored when *auto_pad*
    is specified.

* *dilations*

  * **Description**: *dilations* denotes the distance in width and height
    between elements in the filter. For example, *dilation* equal *1,1* means
    that all the elements in the filter are neighbors, so it is the same as for
    the usual pooling. *dilation* equal *2,2* means that all the elements in the
    filter are matched not to adjacent elements in the input matrix, but to
    those that are adjacent with distance 1.
  * **Range of values**: Non-negative s64 values.
  * **Type**: s64[]
  * **Default value**: list of 1 (no dilation)
  * **Required**: *no*

* *data_format*

  * **Description**: *data_format* denotes the data format of the input,
    output_delta and input_delta.
  * **Range of values**: *NXC* or *NCX* (X means HW for 2D, DHW for 3D)
  * **Type**: string
  * **Default value**: *NXC*
  * **Required**: *no*

## Inputs

* **1**: ``input_forward`` - original input tensor of MaxPool op. **Required.**

  * **Type**: T

* **2**: ``output_delta`` - the gradient tensor with respect to output of
  MaxPool. **Required.**

  * **Type**: T

## Outputs

* **1**: ``input_delta`` - the gradient tensor with respect to the input of
  MaxPool.

  * **Type**: T

**Types**:

* **T**: f32, f16, bf16.
* **Note**: Inputs and outputs have the same data type denoted by *T*. For
  example, if input is f32 tensor, then all other tensors have f32 data type.
