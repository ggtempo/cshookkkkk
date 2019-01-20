#include "visuals.hpp"
#include "../../ImGui/imgui.h"
#include <glad/gl.h>

namespace features
{
    void visuals::studio_render_model(CStudioModelRenderer* ecx)
    {
        static auto& g = globals::instance();
        auto entity = g.engine_studio->GetCurrentEntity();
        auto local = g.engine_funcs->GetLocalPlayer();

        if (entity && entity->model && entity->player && entity != local)
        {
            glDepthRange(0, 0.1);
            glEnable(GL_TEXTURE_2D);
            ecx->StudioRenderFinal();


            glDisable(GL_TEXTURE_2D);
            g.engine_studio->SetForceFaceFlags( g.s_nf );
            g.engine_funcs->pTriAPI->RenderMode( g.render_mode );
            entity->curstate.renderfx = g.render_fx;
            entity->curstate.renderamt = g.fx_amt;
            entity->curstate.rendercolor.r = g.clr.x;
            entity->curstate.rendercolor.g = g.clr.y;
            entity->curstate.rendercolor.b = g.clr.z;
            glColor4f(g.clr.x, g.clr.y, g.clr.z, (float)g.fx_amt / 255.0);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            ecx->StudioRenderFinal();

            glDisable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);

            glPointSize(2.f);

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_TEXTURE_2D);
            glDepthRange(0.1, 1);
        }
        else
        {
            ecx->StudioRenderFinal( );
        }
    }

    void visuals::show_menu()
    {
        ImGui::Begin("ESP");
        ImGui::End();
    }
}