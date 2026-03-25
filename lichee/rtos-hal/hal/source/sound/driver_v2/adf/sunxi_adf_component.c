/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <aw_common.h>
#include <sound_v2/sunxi_adf_core.h>
#include <sound_v2/sunxi_adf_component.h>

int sunxi_sound_adf_component_set_pll(struct sunxi_sound_adf_component *component, int pll_id,
		       int source, unsigned int freq_in, unsigned int freq_out)
{
	int ret = 0;

	if (component->driver->set_pll) {
		ret = component->driver->set_pll(component, pll_id, source,
						  freq_in, freq_out);
		if (ret < 0) {
			snd_err("Adf:error on %s,:%d.\n",
				component->name, ret);
			return ret;
		}
	}
	return 0;
}

int sunxi_sound_adf_component_open(struct sunxi_sound_adf_component *component,
			   struct sunxi_sound_pcm_dataflow *dataflow)
{
	int ret = 0;

	if (component->driver->open) {
		ret = component->driver->open(component, dataflow);
		if (ret < 0) {
			snd_err("Adf:error on %s,:%d.\n",
				component->name, ret);
			return ret;
		}
	}
	return 0;
}

int sunxi_sound_adf_component_close(struct sunxi_sound_adf_component *component,
			   struct sunxi_sound_pcm_dataflow *dataflow)
{
	int ret = 0;

	if (component->driver->close) {
		ret = component->driver->close(component, dataflow);
		if (ret < 0) {
			snd_err("Adf:error on %s,:%d.\n",
				component->name, ret);
			return ret;
		}
	}
	return 0;
}


void sunxi_sound_adf_component_suspend(struct sunxi_sound_adf_component *component)
{
	if (component->driver->suspend)
		component->driver->suspend(component);
	component->suspended = 1;
}

void sunxi_sound_adf_component_resume(struct sunxi_sound_adf_component *component)
{
	if (component->driver->resume)
		component->driver->resume(component);
	component->suspended = 0;
}

int sunxi_sound_adf_component_probe(struct sunxi_sound_adf_component *component)
{
	int ret = 0;

	if (component->driver->probe) {
		ret = component->driver->probe(component);
		if (ret < 0) {
			snd_err("Adf:error on %s,:%d.\n",
				component->name, ret);
			return ret;
		}
	}
	return 0;

}

void sunxi_sound_adf_component_remove(struct sunxi_sound_adf_component *component)
{
	if (component->driver->remove)
		component->driver->remove(component);
}

unsigned int sunxi_sound_adf_component_read_no_lock(struct sunxi_sound_adf_component *component,
	unsigned int reg)
{
	int ret;
	unsigned int val = 0;

	if (component->driver->read) {
		ret = 0;
		val = component->driver->read(component, reg);
	}
	else
		ret = -EIO;

	if (ret < 0) {
		snd_err("Adf:error on %s,:%d.\n",
			component->name, ret);
		return ret;
	}

	return val;
}

int sunxi_sound_adf_component_write_no_lock(struct sunxi_sound_adf_component *component,
		unsigned int reg, unsigned int val)
{
	int ret = -EIO;

	if (component->driver->write)
		ret = component->driver->write(component, reg, val);

	if (ret < 0) {
		snd_err("Adf:error on %s,:%d.\n",
			component->name, ret);
	}
	return ret;
}


snd_pcm_uframes_t sunxi_sound_adf_pcm_component_pointer(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);
	struct sunxi_sound_adf_component *component;
	int i;

	for_each_rtp_components(rtp, i, component)
		if (component->driver->pointer)
			return component->driver->pointer(component, dataflow);

	return 0;
}

int sunxi_sound_adf_pcm_component_ioctl(struct sunxi_sound_pcm_dataflow *dataflow,
				unsigned int cmd, void *arg)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);
	struct sunxi_sound_adf_component *component;
	int i , ret;

	for_each_rtp_components(rtp, i, component)
		if (component->driver->ioctl) {
			ret = component->driver->ioctl(component,
						 dataflow, cmd, arg);
		if (ret < 0) {
			snd_err("Adf:error on %s,:%d.\n",
				component->name, ret);
			return ret;
		}
	}

	return sunxi_sound_pcm_lib_ioctl(dataflow, cmd, arg);
}

int sunxi_sound_adf_pcm_component_new(struct sunxi_sound_adf_pcm_running_param *rtp)
{
	struct sunxi_sound_adf_component *component;
	int ret;
	int i;

	for_each_rtp_components(rtp, i, component) {
		if (component->driver->pcm_construct) {
			ret = component->driver->pcm_construct(component, rtp);
			if (ret < 0) {
				snd_err("Adf:error on %s,:%d.\n",
					component->name, ret);
				return ret;
			}
		}
	}
	return 0;
}

void sunxi_sound_adf_pcm_component_free(struct sunxi_sound_adf_pcm_running_param *rtp)
{
	struct sunxi_sound_adf_component *component;
	int i;

	if (!rtp->pcm)
		return;

	for_each_rtp_components(rtp, i, component)
		if (component->driver->pcm_destruct)
			component->driver->pcm_destruct(component, rtp->pcm);
}

int sunxi_sound_adf_pcm_component_prepare(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);
	struct sunxi_sound_adf_component *component;
	int i, ret;

	for_each_rtp_components(rtp, i, component) {
		if (component->driver->prepare) {
			ret = component->driver->prepare(component, dataflow);
			if (ret < 0) {
				snd_err("Adf:error on %s,:%d.\n",
					component->name, ret);
				return ret;
			}
		}
	}

	return 0;
}


int sunxi_sound_adf_pcm_component_hw_params(struct sunxi_sound_pcm_dataflow *dataflow,
				    struct sunxi_sound_pcm_hw_params *params)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);
	struct sunxi_sound_adf_component *component;
	int i, ret;

	for_each_rtp_components(rtp, i, component) {
		if (component->driver->hw_params) {
			ret = component->driver->hw_params(component,
							   dataflow, params);
			if (ret < 0) {
				snd_err("Adf:error on %s,:%d.\n",
					component->name, ret);
				return ret;
			}
		}
	}

	return 0;
}


void sunxi_sound_adf_pcm_component_hw_free(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);
	struct sunxi_sound_adf_component *component;
	int i, ret;

	for_each_rtp_components(rtp, i, component) {

		if (component->driver->hw_free) {
			ret = component->driver->hw_free(component, dataflow);
			if (ret < 0) {
				snd_err("Adf:error on %s,:%d.\n",
					component->name, ret);
			}
		}
	}
}

int sunxi_sound_adf_pcm_component_trigger(struct sunxi_sound_pcm_dataflow *dataflow, int cmd)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);
	struct sunxi_sound_adf_component *component;
	int i, ret;

	for_each_rtp_components(rtp, i, component) {

		if (component->driver->trigger) {
			ret = component->driver->trigger(component, dataflow, cmd);
			if (ret < 0) {
				snd_err("Adf:error on %s,:%d.\n",
					component->name, ret);
				return ret;
			}
		}
	}
	return 0;
}

int sunxi_sound_adf_pcm_component_dapm_control(struct sunxi_sound_pcm_dataflow *dataflow, int onoff)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);
	struct sunxi_sound_adf_component *component;
	int i, ret;

	for_each_rtp_components(rtp, i, component) {

		if (component->driver->dapm_control) {
			ret = component->driver->dapm_control(component, dataflow, onoff);
			if (ret < 0) {
				snd_err("Adf:error on %s,:%d.\n",
					component->name, ret);
				return ret;
			}
		}
	}
	return 0;
}


