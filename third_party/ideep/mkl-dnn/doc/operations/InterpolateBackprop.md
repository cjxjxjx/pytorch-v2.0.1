# InterpolateBackprop {#dev_guide_op_interpolatebackprop}

**Versioned name**: *InterpolateBackprop-1*

**Category**: *image processing*

**Short description**: Computes the gradients of Interpolate operation.

## Attributes

* *mode*

  * **Description**: specifies type of interpolation
  * **Range of values**: one of ``nearest``, ``linear``, ``bilinear``,
    ``trilinear``
  * **Type**: string
  * **Required**: *yes*

* *coordinate_transformation_mode*

  * **Description**: specifies how to transform the coordinate in the resized
    tensor to the coordinate in the original tensor
  * **Range of values**: name of the transformation mode in string format (here
    ``scale[x]`` is ``output_shape[x] / input_shape[x]`` and ``x_resized`` is a
    coordinate in axis ``x``, for any axis ``x`` from the input ``axes``):

    * ``half_pixel`` - the coordinate in the original tensor axis ``x`` is
      calculated as ``((x_resized + 0.5) / scale[x]) - 0.5``.
    * ``align_corners`` - the coordinate in the original tensor axis ``x`` is
      calculated as ``0 if output_shape[x] == 1 else  x_resized *
      (input_shape[x] - 1) / (output_shape[x] - 1)``.

  * **Type**: string
  * **Default value**: ``half_pixel``
  * **Required**: *no*

* *sizes*

  * **Description**: specifies output shape for spatial axes. *sizes* and
    *scales* can't be valid at the same time. When *sizes* is used, optional
    *scales* should not be set.
  * **Range of values**: positive s64 values
  * **Type**: s64[]
  * **Default value**: none
  * **Required**: *no*

* *scales*

  * **Description**: specifies scales for spatial axes. *sizes* and *scales*
    can't be valid at the same time. When *scales* is used, optional *size*
    should not be set.
  * **Range of values**: f32 values
  * **Type**: f32[]
  * **Default value**: none
  * **Required**: *no*

* *data_format*

  * **Description**: *data_format* denotes the data format of the input and
    output data.
  * **Range of values**: *NXC* or *NCX* (X means HW for 2D, DHW for 3D)
  * **Type**: string
  * **Default value**: *NXC*
  * **Required**: *no*

## Inputs

* **1**: ``input_forward`` - original input tensor of Interpolate op.
  **Required.**

  * **Type**: T1

* **2**: ``output_delta`` - the gradient tensor with respect to the output.
  **Required.**

  * **Type**: T1

* **3**: ``sizes`` - a 1D tensor describing output shape for spatial axes.
  **Optional.**

  * **Type**: T2

## Outputs

* **1**: ``input_delta`` - the gradient tensor with respect to the input of
  Interpolate.

  * **Type**: T1

**Types**:

* **T1**: f32, f16, bf16.
* **T2**: s32.
* **Note**: The input tensor and the result tensor have the same data type
  denoted by *T1*. For example, if input is f32 tensor, then result tensor has
  f32 data type.
