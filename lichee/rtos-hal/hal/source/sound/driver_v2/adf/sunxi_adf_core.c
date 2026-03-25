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
#include <hal_mutex.h>

#include <aw_common.h>
#include <sound_v2/sunxi_adf_core.h>

#ifdef CONFIG_COMPONENTS_PM
#include <pm_devops.h>
#endif

static sunxi_sound_mutex_t sound_core_mutex;
static LIST_HEAD(component_list);

#if defined(CONFIG_SUNXI_SOUND_CODEC_DUMMY)

extern int sunxi_sound_adf_component_is_dummy(struct sunxi_sound_adf_component *component);

#else

int sunxi_sound_adf_component_is_dummy(struct sunxi_sound_adf_component *component)
{
	return 0;
}
#endif

static int sunxi_sound_adf_rtp_add_component(struct sunxi_sound_adf_pcm_running_param *rtp,
				     struct sunxi_sound_adf_component *component)
{
	struct sunxi_sound_adf_component *comp;
	int i;

	for_each_rtp_components(rtp, i, comp) {
		/* already connected */
		if (comp == component)
			return 0;
	}

	/* see for_each_rtd_components */
	rtp->components[rtp->num_components] = component;
	rtp->num_components++;

	return 0;
}

static int sunxi_sound_adf_is_matching_component(
	const struct sunxi_sound_adf_dai_bind_component *dbc,
	struct sunxi_sound_adf_component *component)
{
	if (!dbc)
		return 0;

	if (dbc->name && strcmp(component->name, dbc->name))
		return 0;

	return 1;
}


static struct sunxi_sound_adf_component *sunxi_sound_find_component(
	const struct sunxi_sound_adf_dai_bind_component *dbc)
{
	struct sunxi_sound_adf_component *component;

	list_for_each_entry(component, &component_list, list)
		if (sunxi_sound_adf_is_matching_component(dbc, component))
			return component;

	return NULL;
}


static int sunxi_sound_adf_dai_bind_check(struct sunxi_sound_adf_card *card,
				struct sunxi_sound_adf_dai_bind *dai_bind)
{

	int i;
	struct sunxi_sound_adf_dai_bind_component *cpu, *codec, *platform;

	for_each_bind_codecs(dai_bind, i, codec) {
		if (!codec->name && !codec->bind_name) {
			snd_err("Adf: codec name %s and bind name %s are must be set for one\n",
				codec->name, codec->bind_name);
			return -EINVAL;
		}

		if (!sunxi_sound_find_component(codec)) {
			snd_err("Adf: codec component %s not found for link %s\n",
				codec->name, dai_bind->name);
			return -SUNXI_ERROEPROBE;
		}
	}

	for_each_bind_platforms(dai_bind, i, platform) {
		if (!platform->name /*&& !platform->bind_name*/) {
			snd_err("Adf: platform name %s must be set\n",
				platform->name);
			return -EINVAL;
		}

		if (!sunxi_sound_find_component(platform)) {
			snd_err("Adf: platform component %s not found for link %s\n",
				platform->name, dai_bind->name);
			return -SUNXI_ERROEPROBE;
		}
	}

	for_each_bind_cpus(dai_bind, i, cpu) {
		if (!cpu->name && !cpu->bind_name) {
			snd_err("Adf: cpu name %s and bind name %s are must be set for one\n",
				cpu->name, cpu->bind_name);
			return -EINVAL;
		}

		if (!sunxi_sound_find_component(cpu)) {
			snd_err("Adf: cpu component %s not found for link %s\n",
				cpu->name, dai_bind->name);
			return -SUNXI_ERROEPROBE;
		}
	}
	return 0;
}

void sunxi_sound_adf_destory_pcm_runtime(struct sunxi_sound_adf_pcm_running_param *rtp)
{
	if (!rtp)
		return;

	list_del(&rtp->list);

	sunxi_sound_adf_pcm_component_free(rtp);
}

static struct sunxi_sound_adf_pcm_running_param *_sunxi_sound_adf_create_pcm_running_params (
	struct sunxi_sound_adf_card *card, struct sunxi_sound_adf_dai_bind *dai_bind)
{

	struct sunxi_sound_adf_pcm_running_param *rtp;
	struct sunxi_sound_adf_component *component;

	rtp = sound_malloc(sizeof(*rtp) +
					sizeof(*component) * (dai_bind->num_cpus +
					dai_bind->num_codecs +
					dai_bind->num_platforms));
	if (!rtp) {
		snd_err("Adf: Malloc failed");
		return NULL;
	}
	INIT_LIST_HEAD(&rtp->list);

	rtp->dais = sound_malloc((dai_bind->num_cpus + dai_bind->num_codecs) *
					sizeof(struct sunxi_sound_adf_dai *));
	if (!rtp->dais)
		goto free_rtp;

	rtp->num_cpus	= dai_bind->num_cpus;
	rtp->num_codecs = dai_bind->num_codecs;
	rtp->card	= card;
	rtp->dai_bind	= dai_bind;
	rtp->num	= card->num_rtp++;

	list_add_tail(&rtp->list, &card->rtp_list);

	return rtp;

free_rtp:
	list_del(&rtp->list);
	return NULL;
}

#ifdef CONFIG_COMPONENTS_PM

static int sunxi_sound_adf_suspend(struct pm_device *dev, suspend_mode_t mode)
{
	struct sunxi_sound_adf_card *card = dev->data;
	struct sunxi_sound_adf_pcm_running_param *rtp;
	struct sunxi_sound_adf_component *component;
	int i;

	if (!card->inited)
		return 0;

	/* suspend all pcms */
	for_each_card_rtps(card, rtp) {
		sunxi_sound_pcm_suspend_total(rtp->pcm);
	}

	/* suspend all COMPONENTs */
	for_each_card_rtps(card, rtp) {
		for_each_rtp_components(rtp, i, component) {
			if (component->suspended)
				continue;

			sunxi_sound_adf_component_suspend(component);
		}
	}

	for_each_card_rtps(card, rtp) {
		sunxi_sound_adf_pcm_dapm_control_total(rtp->pcm, 0);
	}

	return 0;
}

static int sunxi_sound_adf_resume(struct pm_device *dev, suspend_mode_t mode)
{
	struct sunxi_sound_adf_card *card = dev->data;
	struct sunxi_sound_adf_pcm_running_param *rtp;
	struct sunxi_sound_adf_component *component;
	int i;

	if (!card->inited)
		return 0;

	for_each_card_rtps(card, rtp) {
		for_each_rtp_components(rtp, i, component) {
			if (component->suspended)
				sunxi_sound_adf_component_resume(component);
		}
	}

	for_each_card_rtps(card, rtp) {
		sunxi_sound_adf_pcm_dapm_control_total(rtp->pcm, 1);
	}

	/* resume all pcms */
	for_each_card_rtps(card, rtp) {
		sunxi_sound_pcm_resume_total(rtp->pcm);
	}

	return 0;
}

static struct pm_devops sunxi_sound_adf_pm_ops = {
	.suspend	= sunxi_sound_adf_suspend,
	.resume		= sunxi_sound_adf_resume,
};

#else
#define sunxi_sound_adf_suspend NULL
#define sunxi_sound_adf_resume NULL
#endif


struct sunxi_sound_adf_dai *sunxi_sound_adf_find_dai(
	const struct sunxi_sound_adf_dai_bind_component *dbc)
{
	struct sunxi_sound_adf_component *component;
	struct sunxi_sound_adf_dai *dai;

	/* Find CPU DAI from registered DAIs */
	list_for_each_entry(component, &component_list, list) {
		if (!sunxi_sound_adf_is_matching_component(dbc, component))
			continue;
		list_for_each_entry(dai, &(component)->bind_list, list) {
			if (dbc->bind_name && strcmp(dai->name, dbc->bind_name)
			    && (!dai->driver->name
				|| strcmp(dai->driver->name, dbc->bind_name)))
				continue;

			return dai;
		}
	}

	return NULL;
}

struct sunxi_sound_adf_component* sunxi_sound_adf_lookup_component_nolocked(const char *driver_name)
{
	struct sunxi_sound_adf_component *component;
	struct sunxi_sound_adf_component *found_component;

	if (!driver_name)
		return NULL;

	found_component = NULL;

	list_for_each_entry(component, &component_list, list) {
		if (strlen(component->driver->name) == strlen(driver_name) &&
			(strcmp(component->driver->name, driver_name) == 0)) {
			found_component = component;
			break;
		}
	}

	return found_component;
}

static int sunxi_sound_adf_create_pcm_running_params(struct sunxi_sound_adf_card *card,
				struct sunxi_sound_adf_dai_bind *dai_bind)
{
	struct sunxi_sound_adf_pcm_running_param *rtp;
	struct sunxi_sound_adf_dai_bind_component *codec, *platform, *cpu;
	struct sunxi_sound_adf_component *component;
	int i, ret;

	snd_debug("Adf: binding %s\n", dai_bind->name);

	ret = sunxi_sound_adf_dai_bind_check(card, dai_bind);
	if (ret < 0)
		return ret;

	rtp = _sunxi_sound_adf_create_pcm_running_params(card, dai_bind);
	if (!rtp)
		return -ENOMEM;

	for_each_bind_cpus(dai_bind, i, cpu) {
		adf_rtp_to_cpu(rtp, i) = sunxi_sound_adf_find_dai(cpu);
		if (!adf_rtp_to_cpu(rtp, i)) {
			snd_info("Adf: CPU DAI %s not registered\n", cpu->bind_name);
			goto _err_defer;
		}
		sunxi_sound_adf_rtp_add_component(rtp, adf_rtp_to_cpu(rtp, i)->component);
	}

	/* Find CODEC from registered CODECs */
	for_each_bind_codecs(dai_bind, i, codec) {
		adf_rtp_to_codec(rtp, i) = sunxi_sound_adf_find_dai(codec);
		if (!adf_rtp_to_codec(rtp, i)) {
			snd_info("Adf: CODEC DAI %s not registered\n", codec->bind_name);
			goto _err_defer;
		}

		sunxi_sound_adf_rtp_add_component(rtp, adf_rtp_to_codec(rtp, i)->component);
	}

	/* Find PLATFORM from registered PLATFORMs */
	for_each_bind_platforms(dai_bind, i, platform) {
		list_for_each_entry(component, &component_list, list)  {
			if (!sunxi_sound_adf_is_matching_component(platform, component))
				continue;

			sunxi_sound_adf_rtp_add_component(rtp, component);
		}
	}

	return 0;

_err_defer:
	sunxi_sound_adf_destory_pcm_runtime(rtp);
	return -SUNXI_ERROEPROBE;
}

static void sunxi_sound_release_card_resources(struct sunxi_sound_adf_card *card)
{
	struct sunxi_sound_adf_component *component;
	struct sunxi_sound_adf_pcm_running_param *rtp, *n;
	int i, ret = 0;

	for_each_card_rtps(card, rtp) {
		/* remove all rtp connected DAIs */
		sunxi_sound_adf_pcm_dai_remove(rtp);
	}

	for_each_card_rtps(card, rtp) {
		for_each_rtp_components(rtp, i, component) {

			if (!component->card)
				return;
			sunxi_sound_adf_component_remove(component);
			component->card = NULL;

		}
	}

	for_each_card_rtps_safe(card, rtp, n) {
		if (!rtp)
			continue;
		sunxi_sound_adf_destory_pcm_runtime(rtp);
	}

	if (card->probed && card->remove) {
		ret = card->remove(card);
		if (ret != 0) {
			snd_err("card remove failed\n");
		}
	}
	card->probed = 0;

	if (card->sound_card) {
		sunxi_sound_card_destory(card->sound_card);
		card->sound_card = NULL;
	}
	snd_info("\n");
}

struct sunxi_sound_adf_control *sunxi_sound_adf_create_control(const struct sunxi_sound_adf_control_new *control, void *data)
{
	struct sunxi_sound_adf_control_new adf_tmp_control;
	struct sunxi_sound_adf_control* adf_control;

	memcpy(&adf_tmp_control, control, sizeof(adf_tmp_control));

	adf_control = sunxi_sound_control_create(&adf_tmp_control, data);

	return adf_control;
}

static int sunxi_sound_add_controls(struct sunxi_sound_card *card,
	const struct sunxi_sound_adf_control_new *controls, int num_controls,
	struct sunxi_sound_adf_component *component)
{
	int i;
	int err;
	struct sunxi_sound_adf_control *adf_control;

	for (i = 0; i < num_controls; i++) {
		const struct sunxi_sound_adf_control_new *control = &controls[i];
		adf_control = sunxi_sound_adf_create_control(control, component);
		err = sound_add_control(card, adf_control);
		if (err < 0) {
			snd_err("Adf: Failed to add %s: %d\n", control->name, err);
			return err;
		}
	}
	return 0;
}

int sunxi_sound_add_component_controls(struct sunxi_sound_adf_component *component,
	const struct sunxi_sound_adf_control_new *controls, unsigned int num_controls)
{
	struct sunxi_sound_card *card = component->card->sound_card;

	return sunxi_sound_add_controls(card, controls, num_controls, component);
}

static int sunxi_sound_adf_running_params_set_dai_fmt(struct sunxi_sound_adf_pcm_running_param *rtp,
						unsigned int dai_fmt)
{
	struct sunxi_sound_adf_dai *cpu_dai;
	struct sunxi_sound_adf_dai *codec_dai;
	unsigned int i;
	int ret;

	for_each_rtp_codec_dais(rtp, i, codec_dai) {
		ret = sunxi_sound_adf_dai_set_fmt(codec_dai, dai_fmt);
		if (ret != 0 && ret != -SUNXI_ENOTSUPP)
			return ret;
	}

	for_each_rtp_cpu_dais(rtp, i, cpu_dai) {
		ret = sunxi_sound_adf_dai_set_fmt(cpu_dai, dai_fmt);
		if (ret != 0 && ret != -SUNXI_ENOTSUPP)
			return ret;
	}
	return 0;
}

static int sunxi_sound_adf_init_pcm_running_params(struct sunxi_sound_adf_card *card,
					struct sunxi_sound_adf_pcm_running_param *rtp)
{
	struct sunxi_sound_adf_dai_bind *dai_bind = rtp->dai_bind;
	int ret, num;

	if (dai_bind->dai_fmt) {
		ret = sunxi_sound_adf_running_params_set_dai_fmt(rtp, dai_bind->dai_fmt);
		if (ret)
			return ret;
	}

	num = rtp->num;

	/* create the pcm */
	ret = sunxi_sound_adf_pcm_create(rtp, num);
	if (ret < 0) {
		snd_err("Adf: can't create pcm %s :%d\n",
			dai_bind->name, ret);
		return ret;
	}

	return 0;
}

static int sunxi_sound_adf_probe_component(struct sunxi_sound_adf_card *card,
					struct sunxi_sound_adf_component *component)
{
	int probed = 0;
	int ret;

	if (sunxi_sound_adf_component_is_dummy(component))
		return 0;

	if (component->card) {
		if (component->card != card) {
			snd_err("Trying to bind component to card \"%s\" but is already bound to card \"%s\"\n",
				card->name, component->card->name);
			return -EINVAL;
		}
		return 0;
	}

	component->card = card;

	ret = sunxi_sound_adf_component_probe(component);
	if (ret < 0)
		goto err;

	probed = 1;

	ret = sunxi_sound_add_component_controls(component,
					     component->driver->controls,
					     component->driver->num_controls);
	if (ret < 0)
		goto err;

err:
	if (probed && ret < 0) {
		if (component->card) {
			sunxi_sound_adf_component_remove(component);
			component->card = NULL;
		}
	}
	return ret;

}

static void sunxi_sound_adf_unlink_cards(struct sunxi_sound_adf_card *card)
{
	if (card->inited) {
		card->inited = false;
		sunxi_sound_release_card_resources(card);
	}
}

static int sunxi_sound_adf_link_cards(struct sunxi_sound_adf_card *card)
{
	struct sunxi_sound_adf_pcm_running_param *rtp;
	struct sunxi_sound_adf_component *component;
	struct sunxi_sound_adf_dai_bind *dai_bind;
	int ret, i;

	sound_mutex_lock(sound_core_mutex);

	card->num_rtp = 0;
	for_each_card_prebinds(card, i, dai_bind) {
		ret = sunxi_sound_adf_create_pcm_running_params(card, dai_bind);
		if (ret < 0) {
			snd_err("Adf: It can't create pcm running for card %s,:%d.\n",
				card->name, ret);
			goto end;
		}
	}

	ret = sunxi_sound_card_create(card->name, &card->sound_card);
	if (ret < 0) {
		snd_err("Adf: It can't create sound card for card %s,:%d.\n",
			card->name, ret);
		goto end;
	}

	if (card->probe) {
		int ret = card->probe(card);

		if (ret < 0) {
			snd_err("Adf:error on %s,:%d.\n",
				card->name, ret);
			goto end;
		}
		card->probed = 1;
	}

	for_each_card_rtps(card, rtp) {
		for_each_rtp_components(rtp, i, component) {

			ret = sunxi_sound_adf_probe_component(card, component);
			if (ret < 0) {
				snd_err("Adf:failed to probe component  %s,:%d.\n",
				component->name, ret);
				goto end;
			}
		}
	}

	for_each_card_rtps(card, rtp) {
		snd_info("Adf: probe %s bind link %d\n",card->name, rtp->num);
		ret = sunxi_sound_adf_pcm_dai_probe(rtp);
		if (ret)
			return ret;
	}

	for_each_card_rtps(card, rtp) {
		ret = sunxi_sound_adf_init_pcm_running_params(card, rtp);
		if (ret < 0)
			goto end;
	}

	ret = sunxi_sound_device_register_all(card->sound_card);
	if (ret < 0) {
		snd_err("Adf: failed to register sound_card %d\n", ret);
		goto end;
	}

#ifdef CONFIG_COMPONENTS_PM
	struct pm_device* pmdev;
	pmdev = (struct pm_device*)sound_malloc(sizeof(struct pm_device));
	pmdev->data = (void *)card;
	pmdev->name = card->name;
	pmdev->ops = &sunxi_sound_adf_pm_ops;
	ret = pm_devops_register(pmdev);
	if (ret) {
		snd_err("pm_devops_register failed\n");
		goto end;
	}
	card->pm_dev = (void *)pmdev;
#endif

	card->inited = 1;

end:
	if (ret < 0)
		sunxi_sound_release_card_resources(card);

	sound_mutex_unlock(sound_core_mutex);

	return ret;

}


int sunxi_sound_adf_register_card(struct sunxi_sound_adf_card *card)
{
	if (!card->name) {
		snd_err("card name is NULL.\n");
		return -EINVAL;
	}

	if (!sound_core_mutex) {
		sound_core_mutex = sound_mutex_init();
		if (sound_core_mutex == NULL) {
			snd_err("hal_mutex_create err.\n");
			return -EINVAL;
		}
	}

	card->pcm_mutex = sound_mutex_init();
	if (card->pcm_mutex == NULL) {
		snd_err("hal_mutex_create err.\n");
		return -EINVAL;
	}

	INIT_LIST_HEAD(&card->rtp_list);

	card->inited = 0;

	return sunxi_sound_adf_link_cards(card);
}

int sunxi_sound_adf_unregister_card(struct sunxi_sound_adf_card *card)
{
	sound_mutex_lock(sound_core_mutex);
	sunxi_sound_adf_unlink_cards(card);
#ifdef CONFIG_COMPONENTS_PM
	int ret;
	struct pm_device* pmdev = (struct pm_device*)card->pm_dev;
	ret = pm_devops_unregister(pmdev);
	if (ret) {
		snd_err("pm_devops_unregister failed\n");
	}
	sound_free(card->pm_dev);
#endif
	sound_mutex_unlock(sound_core_mutex);
	snd_debug("Adf: Unregistered card '%s'\n", card->name);

	return 0;
}

void sunxi_sound_adf_unregister_dai(struct sunxi_sound_adf_dai *dai)
{
	snd_debug("Adf: Unregistered DAI '%s'\n", dai->name);
	if (dai->name) {
		sound_free(dai->name);
	}
	list_del(&dai->list);
}

struct sunxi_sound_adf_dai *sunxi_sound_adf_register_dai(struct sunxi_sound_adf_component *component,
					 struct sunxi_sound_adf_dai_driver *dai_drv)
{
	struct sunxi_sound_adf_dai *dai;

	snd_debug("Adf: register DAI %s\n", dai_drv->name);

	if (dai_drv->name == NULL) {
		snd_err("Adf: adf dai driver name is null\n");
		return NULL;
	}

	dai = sound_malloc(sizeof(*dai));
	if (dai == NULL)
		return NULL;

	dai->name = sunxi_sound_strdup_const(dai_drv->name);
	if (!dai->name) {
		snd_err("Adf: Failed to allocate name\n");
		return NULL;
	}

	if (dai_drv->id)
		dai->id = dai_drv->id;
	else
		dai->id = component->num_binds;

	dai->component = component;
	dai->driver = dai_drv;

	/* see for_each_component_dais */
	list_add_tail(&dai->list, &component->bind_list);
	component->num_binds++;

	snd_debug("Adf: Registered DAI '%s'\n", dai->name);
	return dai;


}


static void sunxi_sound_adf_unregister_dais(struct sunxi_sound_adf_component *component)
{
	struct sunxi_sound_adf_dai *dai, *_dai;

	list_for_each_entry_safe(dai, _dai, &(component)->bind_list, list)
		sunxi_sound_adf_unregister_dai(dai);
}

static int sunxi_sound_adf_register_dais(struct sunxi_sound_adf_component *component,
				 struct sunxi_sound_adf_dai_driver *dai_drv,
				 int count)
{
	struct sunxi_sound_adf_dai *dai;
	unsigned int i;
	int ret;

	for (i = 0; i < count; i++) {
		dai = sunxi_sound_adf_register_dai(component, dai_drv + i);
		if (dai == NULL) {
			ret = -ENOMEM;
			goto err;
		}
	}

	return 0;

err:
	sunxi_sound_adf_unregister_dais(component);

	return ret;
}

static void sunxi_sound_adf_del_component(struct sunxi_sound_adf_component *component)
{
	struct sunxi_sound_adf_card *card = component->card;

	sunxi_sound_adf_unregister_dais(component);

	if (component->name) {
		sound_free(component->name);
	}

	if (card)
		sunxi_sound_adf_unlink_cards(card);

	snd_info("\n");

	list_del(&component->list);

	sound_free(component);
}

int sunxi_sound_adf_component_init(struct sunxi_sound_adf_component *component,
				 const struct sunxi_sound_adf_component_driver *driver)
{
	if (driver->name == NULL) {
		snd_err("Adf: component driver name is null\n");
		return -EINVAL;
	}

	INIT_LIST_HEAD(&component->bind_list);
	INIT_LIST_HEAD(&component->list);

	component->name = sunxi_sound_strdup_const(driver->name);
	if (!component->name) {
		snd_err("Adf: Failed to allocate name\n");
		return -ENOMEM;
	}

	component->driver = driver;

	return 0;
}

int sunxi_sound_adf_add_component(struct sunxi_sound_adf_component *component,
			struct sunxi_sound_adf_dai_driver *dai_drv,
			int num_dai)
{
	int ret;

	sound_mutex_lock(sound_core_mutex);

	ret = sunxi_sound_adf_register_dais(component, dai_drv, num_dai);
	if (ret < 0) {
		snd_err("Adf: Failed to register DAIs: %d\n",
			ret);
		goto err;
	}

	/* see for_each_component */
	list_add(&component->list, &component_list);

err:
	if (ret < 0)
		sunxi_sound_adf_del_component(component);

	sound_mutex_unlock(sound_core_mutex);
	return ret;
}

int sunxi_sound_adf_register_component(const struct sunxi_sound_adf_component_driver *component_driver,
			struct sunxi_sound_adf_dai_driver *dai_drv,
			int num_dai)
{
	struct sunxi_sound_adf_component *component;
	int ret;

	component = sound_malloc(sizeof(*component));
	if (!component)
		return -ENOMEM;

	ret = sunxi_sound_adf_component_init(component, component_driver);
	if (ret < 0)
		return ret;

	return sunxi_sound_adf_add_component(component, dai_drv, num_dai);
}

void sunxi_sound_adf_unregister_component(const struct sunxi_sound_adf_component_driver *component_driver)
{
	sound_mutex_lock(sound_core_mutex);
	while (1) {
		struct sunxi_sound_adf_component *component = sunxi_sound_adf_lookup_component_nolocked(component_driver->name);

		if (!component)
			break;

		sunxi_sound_adf_del_component(component);
	}
	sound_mutex_unlock(sound_core_mutex);
}

int sunxi_sound_adf_find_dai_name(const char *name, int id, const char **dai_name)
{
	struct sunxi_sound_adf_component *pos;
	struct sunxi_sound_adf_dai *dai;
	int ret = -SUNXI_ERROEPROBE;

	sound_mutex_lock(sound_core_mutex);
	list_for_each_entry(pos, &component_list, list)
	{
		if (strcmp(name, pos->name) || !pos->num_binds)
			continue;

		if (id < 0 || id >= pos->num_binds) {
			ret = -EINVAL;
			continue;
		}

		ret = 0;
		list_for_each_entry(dai, &(pos)->bind_list, list) {
			if (id == 0)
				break;
			id--;
		}
		*dai_name = dai->driver->name;
		if (!*dai_name)
			*dai_name = pos->name;
		break;
	}
	sound_mutex_unlock(sound_core_mutex);

	return ret;
}

int sunxi_sound_adf_get_dai_bind_codecs(struct sunxi_sound_adf_dai_bind *dai_bind, int *id)
{
	int index, ret;
	struct sunxi_sound_adf_dai_bind_component *codec;

	for_each_bind_codecs(dai_bind, index, codec) {
		ret = sunxi_sound_adf_find_dai_name(codec->name, id[index], &codec->bind_name);
		if (ret < 0)
			goto err;
	}
	return 0;

err:
	return ret;
}



